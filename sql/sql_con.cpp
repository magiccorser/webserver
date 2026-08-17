#include "sql_con.h"

sqlcon_pool::sqlcon_pool()
{
    free_connum = 0;
    curr_connum = 0;
}

sqlcon_pool::~sqlcon_pool()
{
    destroy_pool();
}

sqlcon_pool *sqlcon_pool::get_instance()
{
    static sqlcon_pool instance;
    return &instance;
}

MYSQL *sqlcon_pool::get_con()
{
    m_sem.wait();
    MYSQL *con = nullptr;
    m_lock.lock();
    con = connection_queue.front();
    connection_queue.pop();
    free_connum--;
    curr_connum++;
    m_lock.unlock();
    return con;
}

void sqlcon_pool::sqlcon_pool_init(int max_conn, std::string host, std::string user, std::string password, std::string db, unsigned int port)
{
    for(int i = 0;i < max_conn; i++){
        MYSQL *con = mysql_init(nullptr);
        if(con == nullptr){
            //写日志,sql连接创建失败
            exit(1);
        }
        con = mysql_real_connect(con, host.c_str(), user.c_str(), password.c_str(), db.c_str(), port, nullptr, 0);
        if(con == nullptr){
            //写日志，连接失败
            exit(1);
        }
        free_connum++;
        connection_queue.push(con);
    }
    curr_connum = free_connum;
    m_sem = sem(free_connum);
}

void sqlcon_pool::destroy_pool()
{
    m_lock.lock();
    for(int i = 0;i < connection_queue.size();i++){
        MYSQL *con = connection_queue.front();
        mysql_close(con);
        connection_queue.pop();
    }
    free_connum = 0;
    curr_connum = 0;
    m_lock.unlock();
}

void sqlcon_pool::release_con(MYSQL *con)
{
    m_lock.lock();
    free_connum++;
    curr_connum--;
    connection_queue.push(con);
    m_lock.unlock();
    m_sem.post();
}

unsigned int sqlcon_pool::get_freenum()
{
    return free_connum;
}
