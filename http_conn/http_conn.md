1. 状态机解析 HTTP 请求（http_conn.cpp:244）：CHECK_STATE_REQUESTLINE → HEADER → CONTENT 逐行解析，配合 parse_line() 处理 \r\n 和缓冲区数据不完整（LINE_OPEN）的情况，支持请求不完整时继续等待数据。
2. 零拷贝响应（http_conn.cpp:413-415, 427）：静态文件用 mmap 映射，再用 writev 把「响应头（m_write_buf）+ 文件内容（m_file_address）」两个 iovec 一次发出，避免把文件拷进用户缓冲区。
3. epoll 集成：m_epollfd 和 m_user_count 是静态成员（所有连接共享），read_once() 区分 LT/ET 触发模式（http_conn.cpp:110-131），ET 下循环读到 EAGAIN。
4. 长连接支持：解析 Connection: keep-alive 存到 m_longer，写完响应后 init() 重置连接继续复用（http_conn.cpp:463-465）。
5. CGI 登录/注册：POST 请求解析 user=xxx&passwd=xxx 表单，配合 MySQL 连接池和全局 users map 做登录验证、注册去重（http_conn.cpp:284-355）。
6. 与定时器/线程池协作：timer_flag、improv 用于配合内核链表定时器做非活跃连接超时关闭。