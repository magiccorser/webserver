

```c
int epoll_create(int size)//创建epoll  size变量没用  返回epoll文件描述符
int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
/* op‌：表示要对目标文件描述符执行的操作
EPOLL_CTL_ADD：向 epoll 实例中添加一个新的文件描述符。
EPOLL_CTL_MOD：修改已存在文件描述符的事件类型。
EPOLL_CTL_DEL：从 epoll 实例中删除一个文件描述符。*/
// fd‌：需要操作的目标文件描述符
/*struct epoll_event {
    uint32_t events;
    epoll_data_t data;
};typedefunion epoll_data {
    void *ptr;
    int fd;           //设置socket文件描述符
    uint32_t u32;
    uint64_t u64;
} epoll_data_t;*/

int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
//成功：返回就绪事件数（0 表示超时）。
```

| 1. 创建     | 分配 `struct eventpoll` 初始化红黑树 `rbr` 初始化就绪链表 `rdllist` | `epoll_create`             |
| :---------- | :----------------------------------------------------------- | -------------------------- |
| **2. 添加** | 创建 `struct epitem` 插入 `rbr` 便于查找 **注册回调到 Socket 等待队列** | `epoll_ctl(ADD)`           |
| **3. 触发** | 网卡数据到达 → Socket 缓冲区 唤醒 Socket 等待队列中的 Epoll 回调 **回调将 `epitem` 链入 `rdllist`** | 内核协议栈 + 驱动          |
| **4. 获取** | 检查 `rdllist` 是否为空 如果为空，进程睡眠 如果不为空，**拷贝 `epitem` 信息到用户空间** | `epoll_wait`               |
| **5. 清理** | 从红黑树移除 `epitem` 从 Socket 等待队列注销回调             | `epoll_ctl(DEL)` / `close` |

详细阅读：https://zhuanlan.zhihu.com/p/17856755436