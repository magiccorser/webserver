#ifndef SQL_CON_H
#define SQL_CON_H

#include <mysql/mysql.h>
#include <queue>
#include <string>
#include "../log/log.h"
#include "../lock/lock.h"

class sqlcon_pool{
private:
    std::queue<MYSQL *> connection_queue;
    unsigned int free_connum;
    unsigned int curr_connum;
    locker m_lock;
    sem m_sem;
    int m_close_log;
public:
    sqlcon_pool();
    ~sqlcon_pool();
    static sqlcon_pool *get_instance();
    MYSQL *get_con();
    void sqlcon_pool_init(int max_conn, std::string host, std::string user, std::string password, std::string db, unsigned int port, int close_log);
    void destroy_pool();
    void release_con(MYSQL *con);
    unsigned int get_freenum();
};

#endif