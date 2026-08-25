1. 状态机解析 HTTP 请求（http_conn.cpp:244）：CHECK_STATE_REQUESTLINE → HEADER → CONTENT 逐行解析，配合 parse_line() 处理 \r\n 和缓冲区数据不完整（LINE_OPEN）的情况，支持请求不完整时继续等待数据。
2. 零拷贝响应（http_conn.cpp:413-415, 427）：静态文件用 mmap 映射，再用 writev 把「响应头（m_write_buf）+ 文件内容（m_file_address）」两个 iovec 一次发出，避免把文件拷进用户缓冲区。
3. epoll 集成：m_epollfd 和 m_user_count 是静态成员（所有连接共享），read_once() 区分 LT/ET 触发模式（http_conn.cpp:110-131），ET 下循环读到 EAGAIN。
4. 长连接支持：解析 Connection: keep-alive 存到 m_longer，写完响应后 init() 重置连接继续复用（http_conn.cpp:463-465）。
5. CGI 登录/注册：POST 请求解析 user=xxx&passwd=xxx 表单，配合 MySQL 连接池和全局 users map 做登录验证、注册去重（http_conn.cpp:284-355）。
6. 与定时器/线程池协作：timer_flag、improv 用于配合内核链表定时器做非活跃连接超时关闭。

请求报文 = 请求行 + 请求头 + 空行 + 请求体
GET /index.html HTTP/1.1      ← 请求行：方法  URL  版本
Host: example.com             ← 请求头
Connection: keep-alive
                              ← 空行
username=aaa&passwd=bbb       ← 请求体（POST 才有）
响应报文 = 状态行 + 响应头 + 空行 + 响应体
HTTP/1.1 200 OK               ← 状态行：版本 状态码 原因短语
Content-Type: text/html       ← 响应头
Content-Length: 123
                              ← 空行
<html>...</html>              ← 响应体

第一部分：HTTP 协议知识

1. HTTP 是什么
  HTTP（HyperText Transfer Protocol）是应用层协议，运行在 TCP 之上。模型是请求/响应（request/response）：客户端发一个请求报文，服务器回一个响应报文，一问一答。默认端口 80，HTTPS 是 443（多了 TLS 加密层）。
2. 报文通用格式
  任何 HTTP 报文都由四部分组成，顺序固定：
  起始行(start line)
  头部(header)字段，每行一个
  空行(CRLF)
  消息体(body)，可选
- 请求报文的起始行叫请求行
- 响应报文的起始行叫状态行
- 头部与体之间必须有且仅有一个空行 \r\n 分隔
3. 请求报文结构
  请求行
  请求头
  空行
  请求体(可选)
  请求行格式：
  方法 空格 请求目标(URL) 空格 协议版本 CRLF
  例：GET /index.html HTTP/1.1
  请求头：字段名: 值 形式，每行一条，如 Host: a.com。常见字段见第 7 节。
  请求体：POST 提交表单、上传文件时才有，GET 一般没有。
4. 响应报文结构
  状态行
  响应头
  空行
  响应体
  状态行格式：
  协议版本 空格 状态码 空格 原因短语 CRLF
  例：HTTP/1.1 200 OK
5. 请求方法（Method）
- GET：获取资源，参数附在 URL 后（?a=1&b=2），无请求体
- POST：提交数据，数据放请求体
- HEAD：同 GET 但只返回头不返回体
- PUT：上传/替换资源
- DELETE：删除资源
- OPTIONS：询问服务器支持哪些方法
- CONNECT：用于 HTTPS 隧道
- PATCH：部分修改资源
6. 状态码（Status Code）
  三位数字，首位分类：
- 1xx：信息，继续（如 100 Continue）
- 2xx：成功（200 OK、201 Created）
- 3xx：重定向（301 永久、302 临时、304 未修改）
- 4xx：客户端错误（400 语法错、403 禁止、404 未找到、405 方法不允许）
- 5xx：服务器错误（500 内部错、502 网关错、503 不可用）
7. 常见头部字段
  通用头（请求响应都用）：Date、Cache-Control、Connection、Content-Length、Content-Type
  请求头：Host(HTTP/1.1 必填)、User-Agent、Accept(可接受类型)、Accept-Encoding(gzip等)、Cookie、Authorization、Referer、Range(断点)、Expect
  响应头：Server、Set-Cookie、Location(重定向用)、Content-Disposition、Last-Modified、ETag、WWW-Authenticate
8. URL / URI
  http://host:port/path?query#fragment
- URI 是统一资源标识符，URL 是带位置的 URI
- 请求行里的请求目标通常是 path?query 部分
9. 连接管理
- HTTP/1.0 默认短连接：每请求完就关 TCP
- HTTP/1.1 默认长连接：Connection: keep-alive，一个 TCP 连发多个请求，靠 Content-Length 或 Transfer-Encoding: chunked 界定每个报文边界
- Content-Length 缺失且非 chunked 时，服务器靠「连接关闭」判断体结束
10. 内容协商与编码
- Content-Type 声明体类型：text/html、application/json、image/png 等，可带 charset=utf-8
- Content-Encoding: gzip 表示体被压缩
- Transfer-Encoding: chunked：分块传输，每块格式 长度\r\n数据\r\n，以 0\r\n\r\n 结束
11. 缓存机制
- Cache-Control: max-age=3600
- 条件请求：If-Modified-Since / If-None-Match；服务器比对后回 304 Not Modified 省流量
  第二部分：程序编写套路（怎么写一个 HTTP 解析器）
  套路一：数据结构设计
  一个连接 = 一个对象，对象里保存：
- sockfd：这个连接的 socket
- 读缓冲区 read_buf[N] + 两个下标 read_idx(已收字节数)、checked_idx(已扫描位置)
- start_line：当前行起点
- check_state：当前解析阶段（请求行/头/体）
- method、url、version、content_length、keep_alive 等解析结果
- 写缓冲区 write_buf + write_idx
- 发送进度 bytes_to_send、bytes_have_send
  套路二：解析行的函数（parse_line）
  任务：从 checked_idx 扫到 read_idx，找 \r\n。
- 找到 \r\n：把这两个字节改成 \0，返回「行完整」
- 遇到 \r 但后面还没收到 \n：返回「数据不全」
- 扫完没找到：返回「数据不全」
  作用：把缓冲区切成一条条以 \0 结尾的字符串，供后续解析。
  套路三：主解析循环（状态机核心）
  循环：
    调 parse_line 取一行
    若取不到完整行 → 跳出循环，返回"请求未完成"
    根据 check_state 分流：
        状态=请求行 → parse_request_line()
        状态=头     → parse_headers()
        状态=体     → parse_content()
    若某步返回"已完成" → 调 do_request() 处理并生成响应
  返回"未完成"（等下次数据到来继续）
  关键点：每次只处理「已收到的完整行」，数据不够就停下来保留进度，下次recv到新数据接着跑。这就是状态机跨多次 recv 工作的原理。
  套路四：三个解析处理函数
- parse_request_line：用空格把行拆成 方法 / URL / 版本；校验方法合法性、版本前缀、URL 以 / 开头；设 check_state=头
- parse_headers：逐行判断字段名。空行=头结束：若有 Content-Length>0 转「体」状态，否则直接完成。识别 Host/Connection/Content-Length 存入对应变量
- parse_content：判断已收字节是否 ≥ content_length，够则把体存下，标记完成
  套路五：do_request（业务处理）
- 拼接本地文件路径 doc_root + url
- stat() 检查文件存在/权限/非目录，否则返回对应错误码
- 打开文件 mmap 到内存，准备零拷贝发送
- 返回「文件就绪」或各种错误码
  套路六：生成响应（process_write）
  按 do_request 返回码拼响应：
- 错误码：状态行 + Content-Length + Connection + 错误正文
- 文件：状态行 200 OK + 头 + 用 iovec 把「响应头 + 文件映射地址」组一起，方便 writev 一次发
  套路七：发送与 ET/ONESHOT 配合
- writev 发送；返回 EAGAIN（缓冲满）就 modfd(EPOLLOUT) 等可写事件再来
- 发完：长连接就 init() 重置对象复用，短连接就 close_conn()
- 每次处理完用 EPOLLONESHOT 需重新 modfd 武装，保证线程安全
  套路八：主循环调度（epoll）
  epoll_wait 返回事件：
    listenfd 可读 → accept 新连接，init 注册
    pipe 可读     → 处理信号（alarm/timer）
    某fd EPOLLIN  → 读数据 + process()解析
    某fd EPOLLOUT → 继续 write()发送
    错误/对端关闭 → 关连接 + 删定时器