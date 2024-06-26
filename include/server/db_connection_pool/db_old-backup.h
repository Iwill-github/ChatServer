#ifndef DB_OLD_BACKUP_H
#define DB_OLD_BACKUP_H

#include <mysql/mysql.h>
#include <muduo/base/Logging.h>
#include <string>

// 数据库配置信息
static std::string server = "127.0.0.1";
static std::string user = "wly";
static std::string password = "123456";
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