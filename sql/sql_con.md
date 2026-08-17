## 核心数据类

| 类型         | 是什么                                 |
| :----------- | -------------------------------------- |
| MYSQL        | 连接句柄，代表一条到服务器的连接       |
| MYSQL_RES    | 查询结果集（所有数据行都装在里面）     |
| MYSQL_ROW    | 一行数据，本质是char**（一维数组）     |
| MYSQL_FIELD  | 字段元信息（列名、类型等）             |
| my_ulonglong | 无符号大整数（行数、受影响行数都用它） |

## 执行 SQL 语句

```
// 初始化：分配并初始化一个连接对象
MYSQL *mysql_init(MYSQL *mysql);
// 传 NULL 会自动帮你分配；也可以先建对象再传入
// 失败返回 NULL

//真正建立连接
MYSQL *mysql_real_connect(
    MYSQL *mysql,      // 上面 init 出来的句柄
    const char *host,  // 主机，如 "127.0.0.1" 或 "localhost"
    const char *user,  // 用户名，如 "root"
    const char *passwd,// 密码
    const char *db,    // 要用的数据库名，如 "webserver"，可传 NULL 稍后选
    unsigned int port, // 端口，3306；传 0 用默认
    const char *unix_socket, // 传 NULL 即可
    unsigned long clientflag // 标志位，一般传 0
);
// 成功返回同一个 mysql 指针，失败返回 NULL

// 关闭连接并释放资源
void mysql_close(MYSQL *mysql);

// 最常用：执行一条 SQL（不加分号）
int mysql_query(MYSQL *mysql, const char *q);
// 成功返回 0，失败返回非 0

// 带长度的版本，字符串里可含二进制数据
int mysql_real_query(MYSQL *mysql, const char *q, unsigned long length);
// 例子：建表、插入、更新、删除都用它
mysql_query(conn, "CREATE TABLE IF NOT EXISTS user(id INT PRIMARY KEY AUTO_INCREMENT, name VARCHAR(50), age INT)");
mysql_query(conn, "INSERT INTO user(name, age) VALUES('tom', 20)");
mysql_query(conn, "UPDATE user SET age = 21 WHERE name = 'tom'");
mysql_query(conn, "DELETE FROM user WHERE name = 'tom'");
```

## 取结果集（查询类语句才需要）

```
// 把结果一次性全部搬到内存里（推荐，简单）
MYSQL_RES *mysql_store_result(MYSQL *mysql);
// 失败返回 NULL

// 逐行从服务器取（省内存，但取数据期间连接被占用）
MYSQL_RES *mysql_use_result(MYSQL *mysql);
小白用 mysql_store_result 就够，两个的区别后面讲。
// 拿到一行数据（每调用一次前进一行），取完返回 NULL
MYSQL_ROW mysql_fetch_row(MYSQL_RES *result);

// 结果集行数 / 列数
my_ulonglong mysql_num_rows(MYSQL_RES *result);
unsigned int mysql_num_fields(MYSQL_RES *result);

// 一定要记得释放结果集！
void mysql_free_result(MYSQL_RES *result);
注意：mysql_store_result 出来的结果是字符串数组，不管数据库里是 int 还是 varchar，拿到的都是字符串，需要用 atoi() 转成数字。
```

## 获取字段信息（列名等）

```
// 返回字段数组，每个元素是一个 MYSQL_FIELD
MYSQL_FIELD *mysql_fetch_fields(MYSQL_RES *result);
// 用法：
// field[i].name    列名
// field[i].type    列类型（MYSQL_TYPE_LONG 等）
// field[i].max_length 字段最大长度
```

mysql版本:
mysql  Ver 8.0.46-0ubuntu0.22.04.3 for Linux on x86_64 ((Ubuntu))

安装命令:
sudo apt install mysql-client           //安装mysql客户端
sudo apt install libmysqlclient-dev     //c++库头文件