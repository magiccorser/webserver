#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <iostream>
#include <string>
#include <stdarg.h>
#include <pthread.h>
#include "./blockqueue.h"

class log{
private:
    FILE *m_fp;
    blockqueue<std::string> *m_log_queue;
    locker m_mutex;
    char dir_name[128];
    char log_name[128];
    int m_split_lines;
    int m_log_buf_size;
    long long m_count;
    int m_today; 
    char *m_buf;
    bool m_is_sync;
    int m_close_log;

    log();
    virtual ~log();
    void *async_write_log()
    {
        std::string single_log;
        //从阻塞队列中取出一个日志string，写入文件
        while (m_log_queue->pop(single_log))
        {
            m_mutex.lock();
            fputs(single_log.c_str(), m_fp);
            m_mutex.unlock();
        }
    }

public:
    static log *get_instance()
    {
        static log instance;
        return &instance;
    }

    static void *write_log_thread(void *args)
    {
        log::get_instance()->async_write_log();
    }
    bool init(const char *file_name, int close_log, int log_buf_size = 8192, int split_lines = 5000000, int max_queue_size = 0);

    void write_log(int level, const char *format, ...);

    void flush(void);

};

#define LOG_DEBUG(format, ...) if(0 == m_close_log) {log::get_instance()->write_log(0, format, ##__VA_ARGS__); log::get_instance()->flush();}
#define LOG_INFO(format, ...) if(0 == m_close_log) {log::get_instance()->write_log(1, format, ##__VA_ARGS__); log::get_instance()->flush();}
#define LOG_WARN(format, ...) if(0 == m_close_log) {log::get_instance()->write_log(2, format, ##__VA_ARGS__); log::get_instance()->flush();}
#define LOG_ERROR(format, ...) if(0 == m_close_log) {log::get_instance()->write_log(3, format, ##__VA_ARGS__); log::get_instance()->flush();}

#endif