#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <cassert>
#include <sys/epoll.h>
#include <string.h>

#include "./http_conn/http_conn.h"
#include "./timer/list_timer.h"

#define TIMESLOT            5
#define MAX_FD              1024
#define MAX_EVENT_NUMBER    1024

class webserver{
private:
    int m_listenfd;
    utils m_utils;
    int m_close_log;
public:
    int m_epollfd;
    int m_pipefd[2];
    void init();
    void create_listen();
    void add_to_listtimer(int sockfd, struct sockaddr_in addr);
    bool epoll_run();
    void epoll_dealevent();
};

#endif