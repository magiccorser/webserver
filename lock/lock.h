#ifndef LOCK_H
#define LOCK_H

#include <pthread.h>
#include <semaphore.h>
#include <exception>

class locker
{
private:
    pthread_mutex_t m_mutex;
public:
    locker()
    {
        if(pthread_mutex_init(&m_mutex, nullptr) != 0){
            throw std::exception();
        }
    }
    ~locker()
    {
        pthread_mutex_destroy(&m_mutex);
    }
    bool lock()
    {
        return pthread_mutex_lock(&m_mutex);
    }
    bool locked()
    {
        return pthread_mutex_unlock(&m_mutex);
    }
    pthread_mutex_t *get_lock(){
        return &m_mutex;
    }
};

class sem{
private:
    sem_t m_sem_t;
public:
    sem()
    {
        if(sem_init(&m_sem_t, 0, 1) != 0){
            throw std::exception();
        }
    }
    sem(int num){
        if(sem_init(&m_sem_t, 0, num) != 0){
            throw std::exception();
        }
    }
    ~sem(){
        sem_destroy(&m_sem_t);
    }
    bool wait(){
        return sem_wait(&m_sem_t) == 0;
    }
    bool post(){
        return sem_post(&m_sem_t) == 0;
    }
};

class cond{
private:
    pthread_cond_t m_cond;
public:
    cond(){
        if(pthread_cond_init(&m_cond, NULL) != 0){/*attr	条件变量属性指针，传 NULL 表示使用默认属性（推荐）
                                            若传 attr，则需先用 pthread_condattr_init 初始化属性对象，常用属性：
                                            - pthread_condattr_setclock()：设置等待时使用的时钟（如 CLOCK_MONOTONIC），影响 pthread_cond_timedwait 的超时计算。*/
            throw std::exception();
        }
    }
    ~cond(){
        pthread_cond_destroy(&m_cond);
    }
    bool wait(pthread_mutex_t *mutex){
        return pthread_cond_wait(&m_cond, mutex) == 0;
    }
    bool signal(){
        return pthread_cond_signal(&m_cond) == 0;
    }
    bool boardcast(){
        return pthread_cond_broadcast(&m_cond) == 0;
    }
    bool timedwait(pthread_mutex_t *mutex, const struct timespec *abstime){
        return pthread_cond_timedwait(&m_cond, mutex, abstime) == 0;
    }
};

#endif 