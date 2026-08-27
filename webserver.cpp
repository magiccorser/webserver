#include "webserver.h"

void webserver::init()
{
}

void webserver::create_listen()
{
    int ret;
    m_listenfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(m_listenfd >= 0);

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8888);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    ret = bind(m_listenfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    assert(ret >= 0);
    ret = listen(m_listenfd, 5);
    assert(ret >= 0);
    m_epollfd = epoll_create(5);
    assert(m_epollfd != -1);

    m_utils.init(TIMESLOT);
    m_utils.setnoblocking(m_listenfd);
    m_utils.addfd(m_epollfd, m_listenfd);

    ret = socketpair(AF_UNIX, SOCK_STREAM, 0, m_pipefd);
    assert(ret != -1);
    m_utils.setnoblocking(m_pipefd[1]);        // 写端非阻塞
    m_utils.addfd(m_epollfd, m_pipefd[0]);     // 读端加入 epoll（带 ET+ONESHOT）
    utils::u_epollfd = m_epollfd;
    utils::u_pipefd = m_pipefd;
    // 注册信号
    m_utils.addsig(SIGPIPE, SIG_IGN);
    m_utils.addsig(SIGALRM, utils::sig_handler, false);
    m_utils.addsig(SIGTERM, utils::sig_handler, false);

    // 启动定时心跳
    alarm(TIMESLOT);
}

void webserver::add_to_listtimer(int sockfd, sockaddr_in addr)
{
    util_timer *timer;
    timer->data->address = addr;
    timer->data->sockfd = sockfd;
    timer->func = func;
    time_t cur = time(NULL);
    timer->time = cur + 3 * TIMESLOT;
    m_utils.timer_list.add_timer(timer);
}

bool webserver::epoll_run()
{
    struct sockaddr_in client_addr;
    socklen_t socklen = sizeof(client_addr);
    while(1){
        int acceptfd = accept(m_listenfd, (sockaddr *)&client_addr, &socklen);
        if(acceptfd < 0){
            LOG_ERROR("%s:errno is:%d", "accept error", errno);
            break;
        }
        else if(http_conn::m_user_count > MAX_FD){
            LOG_ERROR("%s", "Internal server busy");
            break;
        }
        add_to_listtimer(acceptfd, client_addr);
    }
    return true;
}

void webserver::epoll_dealevent()
{
    epoll_event events[MAX_EVENT_NUMBER];
    while(1){
        int number = epoll_wait(m_epollfd, events, MAX_EVENT_NUMBER, -1);
        if(number < 0 && errno != EINTR){
            LOG_ERROR("%s", "epoll failure");
            break;
        }
        for(int i = 0;i < number;i++){
            int sockfd = events[i].data.fd;
            if(sockfd == m_listenfd){
                //说明有新的连接，处理新连接
            }
            else if(events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)){
                //连接出错，关闭连接，删掉计时器
                m_utils.removefd(m_epollfd, sockfd);
                util_timer *timer = nullptr;
                m_utils.timer_list.del_timer(timer);
            }else if((sockfd == m_pipefd[0]) && (events[i].events & EPOLLIN)){
                //读信号管道，执行tick()清理超时连接
            }else if(events[i].events & EPOLLIN){
                //读客户端数据
            }else if(events[i].events & EPOLLOUT){
                //往客户端写数据
            }
        }
    }
}
