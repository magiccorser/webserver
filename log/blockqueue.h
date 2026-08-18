#ifndef BLOCKQUEUE_H
#define BLOCKQUEUE_H

#include "../lock/lock.h"

template <typename T>
class blockqueue{
private:
    T array[1000];
    cond m_cond;
    locker m_lock;
    int m_front;
    int m_back;
    int m_size;
public:
    blockqueue(){
        m_front = -1;
        m_back = -1;
        m_size = 0;
    }
    ~blockqueue(){}
    bool isEmpty(){
        m_lock.lock();
        if(m_size == 0){
            m_lock.unlock();
            return true;
        }
        m_lock.unlock();
        return false;
    }
    bool isFull(){
        m_lock.lock();
        if(m_size == 1000){
            m_lock.unlock();
            return true;
        }
        m_lock.unlock();
        return false;
    }
    bool push(const T &item){
        m_lock.lock();
        if(m_size >= 1000){
            m_lock.unlock();
            m_cond.broadcast();
            return false;
        }
        m_back = (m_back + 1) % 1000;
        array[m_back] = item;
        m_size++;
        m_lock.unlock();
        m_cond.broadcast();
        return true;
    }
    bool pop(T &item){
        m_lock.lock();
        while(m_size <= 0){
            if(!m_cond.wait(m_lock.get_lock())){
                m_lock.unlock();
                return false;
            }
        }
        m_front = (m_front + 1) % 1000;
        item = array[m_front];
        m_size--;
        m_lock.unlock();
        return true;
    }
    int size(){
        int tmp = 0;
        m_lock.lock();
        tmp = m_size;
        m_lock.unlock();
        return tmp;
    }
    int max_size(){
        return 1000;
    }
};

#endif