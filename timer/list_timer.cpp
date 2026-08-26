#include "list_timer.h"
#include "../http_conn/http_conn.h"


sort_timer_list::sort_timer_list()
{
    head = nullptr;
    tail = nullptr;
}

sort_timer_list::~sort_timer_list()
{
    util_timer *tmp = head;
    while (tmp)
    {
        head = tmp->next;
        delete tmp;
        tmp = head;
    }
}

void sort_timer_list::add_timer(util_timer *timer)
{
    if(!timer){
        return ;
    }
    if(!head){
        head = timer;
        tail = timer;
        return ;
    }
    if(timer->time < head->time){
        timer->next = head;
        head->prev = timer;
        head = timer;
    }
    add_timer(timer, head);
}

void sort_timer_list::add_timer(util_timer *timer, util_timer *head)
{
    util_timer *tem = head->next;
    while(tem){
        if(tem->time > timer->time){
            util_timer *tmp = tem->prev;
            tmp->next = timer;
            timer->next = tem;
            tem->prev = timer;
            timer->prev = tmp;
            break;
        }
        tem = tem->next;
    }
    if(!tem){
        tail->next = timer;
        timer->prev = tail;
        tail = timer;
    }
}

void sort_timer_list::del_timer(util_timer *timer)
{
    if(!timer){
        return ;
    }
    if((timer == head)&&(timer == tail)){
        delete timer;
        head = nullptr;
        tail = nullptr;
        return ;
    }
    if(timer == head){
        head = head->next;
        head->prev = nullptr;
        delete timer;
        return ;
    }
    if(timer == tail){
        tail = tail->prev;
        tail->next = nullptr;
        delete timer;
    }
    timer->prev->next = timer->next;
    timer->next->prev = timer->prev;
    delete timer;
}

void sort_timer_list::adjust_timer(util_timer *timer)
{
    if(!timer){
        return ;
    }
    util_timer *tem = timer->next;
    if(!tem||timer->time > tem->time){
        return ;
    }
    if(timer == head){
        head = head->next;
        head->prev = nullptr;
        timer->next = nullptr;
        add_timer(timer, head);
    }else{
        timer->prev->next = timer->next;
        timer->next->prev = timer->prev;
        add_timer(timer, timer->next);
    }
}

void sort_timer_list::tick()
{
    time_t cur = time(nullptr);
    util_timer *tem = head;
    while(tem){
        if(cur < tem->time){
            break;
        }
        util_timer *tmp = tem;
        tmp->func(tmp->data);
        head = tem->next;
        if (head) head->prev = nullptr;
        delete tmp;
        tem = head;
    }
}

void utils::init(int timeslot)
{
    m_TIMESLOT = timeslot;
}

int utils::setnoblocking(int fd)
{
    int old_flags = fcntl(fd, F_GETFL);
    int new_flags = old_flags | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_flags);
    return old_flags;
}

void utils::addfd(int epollfd, int fd)
{
    epoll_event ev;
    ev.data.fd = fd;
    ev.events = EPOLLIN | EPOLLRDHUP | EPOLLONESHOT | EPOLLET;
    setnoblocking(fd);
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
}

void utils::modfd(int epollfd, int fd, int ev)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = ev | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
}

void utils::removefd(int epollfd, int fd)
{
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, 0);
    close(fd);
}

void utils::sig_handler(int sig)
{
    int save_errno = errno;
    int msg = sig;
    send(u_pipefd[1], (char *)&msg, 1, 0);
    errno = save_errno;
}

void utils::addsig(int sig, void(handler)(int), bool restart)
{
    struct sigaction sa;                    //Linux 信号注册结构体。
    memset(&sa, '\0', sizeof(sa));          
    sa.sa_handler = handler;                //绑定信号到来时调用的函数（就是上面的sig_handler
    if(restart)
        sa.sa_flags |= SA_RESTART;          //被信号中断的系统调用自动重启（如 recv、accept），防止服务器报错退出
    sigfillset(&sa.sa_mask);                //在处理信号时屏蔽所有其他信号，防止信号嵌套混乱
    assert(sigaction(sig, &sa, NULL) != -1);
}

void utils::timer_handler()
{
    timer_list.tick();
    alarm(m_TIMESLOT);
}

void utils::show_error(int connfd, const char *info)
{
    send(connfd, info, sizeof(info), 0);
    close(connfd);
}

int *utils::u_pipefd = 0;
int utils::u_epollfd = 0;

class utils;
void func(client_data *user_data)
{
    epoll_ctl(utils::u_epollfd, EPOLL_CTL_DEL, user_data->sockfd, 0);
    assert(user_data);
    close(user_data->sockfd);
    http_conn::m_user_count--;
}