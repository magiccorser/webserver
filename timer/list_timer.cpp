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