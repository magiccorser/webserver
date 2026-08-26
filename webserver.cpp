#include "webserver.h"

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