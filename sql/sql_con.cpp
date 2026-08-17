#include "sql_con.h"

sqlcon_pool::sqlcon_pool(int max_conn)
{
    for(int i = 0;i < max_conn; i++){
        MYSQL *con = mysql_init(nullptr);
        if(con == nullptr){
            //写日志,sql连接创建失败
            exit(1);
        }
        this->connection_queue.push(con);
        mysql_real_connect(con, loc);
    }
}
