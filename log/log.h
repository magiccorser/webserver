#ifndef LOG_H
#define LOG_H

#include "../lock/lock.h"
#include "blockqueue.h"
#include <string>

class log{
private:
    blockqueue<std::string> log_queue;
    int m_size;
    int m_max_size;
public:
    log();
    ~log();
    static log *get_instance();
    void log_write(const std::string &msg);
    int log_size();
    
};

#endif