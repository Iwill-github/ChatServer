#ifndef DB_HPP
#define DB_HPP

#include <mysql/mysql.h>
#include <muduo/base/Logging.h>
#include <string>

// 数据库配置信息
static std::string server = "192.168.48.129";
static std::string user = "wly";
static std::string password = "nyjzdl9758&";
static std::string dbname = "chat";
// 数据库操作类

class MySQL{ 
public:
    // 初始化数据库连接
    MySQL();
    ~MySQL();

    // 连接数据库
    bool connect();
    // 更新操作
    bool update(std::string sql);
    // 查询操作
    MYSQL_RES* query(std::string sql);
    // 获取连接
    MYSQL* getConnection();
    
private:
    MYSQL *_conn;
};

#endif