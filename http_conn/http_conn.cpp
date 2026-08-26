#include "http_conn.h"
#include "../log/log.h"
#include <map>

std::vector<std::pair<std::string, std::string>> user;
std::vector<std::string> device;
std::map<int, int> about;
static int m_user_count = 0;

void http_conn::init_mysql(sqlcon_pool *connection)
{
    MYSQL *con = connection->get_con();
    if(mysql_query(con, "select username,password from user") != 0){
        LOG_ERROR("SELECT error:%s\n", mysql_error(con));
    }
    MYSQL_RES *result = mysql_store_result(con);
    MYSQL_ROW row;
    while((row = mysql_fetch_row(result)) != NULL){
        user.push_back({row[0], row[1]});
    }
    mysql_free_result(result);
    if(mysql_query(con, "select qrcode from device") != 0){
        LOG_ERROR("SELECT error:%s\n", mysql_error(con));
    }
    result = mysql_store_result(con);
    while((row = mysql_fetch_row(result)) != NULL){
        device.push_back(row[0]);
    }
    mysql_free_result(result);
    if(mysql_query(con, "select user_id,device_id from user_device") != 0){
        LOG_ERROR("SELECT error:%s\n", mysql_error(con));
    }
    result = mysql_store_result(con);
    while((row = mysql_fetch_row(result)) != NULL){
        about[atoi(row[0])] = atoi(row[1]);
    }
    mysql_free_result(result);
}
