#ifndef SQL_CON_H
#define SQL_CON_H

#include <mysql/mysql.h>
#include <queue>

class sqlcon_pool{
private:
    MYSQL *con;
    
    std::queue<MYSQL *> connection_queue;
public:
    sqlcon_pool(int max_conn);
    ~sqlcon_pool();
    MYSQL *get_con();
};

#endif