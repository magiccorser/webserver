#ifndef HTTP_CONN_H
#define HTTP_CONN_H

#include "../sql/sql_con.h"

class http_conn{
private:

public:
    int m_close_log;
    static int m_user_count;
    void init_mysql(sqlcon_pool *connection);
                                  
};

#endif
