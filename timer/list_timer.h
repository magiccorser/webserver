#ifndef LIST_TIMER_H
#define LIST_TIMER_H

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/uio.h>

#include <time.h>

class util_timer;

struct client_data{
    sockaddr_in address;
    int sockfd;
    util_timer *timer;
};

class util_timer{
public:
    time_t time;
    util_timer *prev;
    util_timer *next;
    util_timer():prev(nullptr), next(nullptr){};
    client_data *data;
    void (*func)(client_data* );
};

class sort_timer_list{
private:
    util_timer *head;
    util_timer *tail;
    void add_timer(util_timer *timer, util_timer *head);

public:
    sort_timer_list();
    ~sort_timer_list();
    void add_timer(util_timer *timer);
    void del_timer(util_timer *timer);
    void adjust_timer(util_timer *timer);
    void tick();
};

class utils{
private:
    int m_TIMESLOT;

public:
    sort_timer_list timer_list;
    static int *u_pipefd;
    static int u_epollfd;
    utils() {}
    ~utils() {}
    void init(int timeslot);
    int setnoblocking(int fd);
    //将内核事件表注册读事件，ET模式，选择开启EPOLLONESHOT
    void addfd(int epollfd, int fd);
    void modfd(int epollfd, int fd, int ev);
    void removefd(int epollfd, int fd);
    static void sig_handler(int sig);
    void addsig(int sig, void(handler)(int), bool restart = true);
    void timer_handler();
    void show_error(int connfd, const char *info);
};

void func(client_data *user_data);
#endif