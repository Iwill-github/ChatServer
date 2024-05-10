/*
    数据库的增删改查
*/

#pragma once
#include <iostream>
#include <mysql/mysql.h>
#include <string>
#include <ctime>

#include <muduo/base/Logging.h>     // muduo 日志模块

using namespace std;

class MySQLConnection {
public:
    // 初始化数据库连接
    MySQLConnection();

    // 释放数据库连接资源
    ~MySQLConnection();

    // 连接数据库
    bool connect(string ip, unsigned short port, string user, string password, string dbname);

    // 更新操作 insert、delete、update
    bool update(string sql);

    // 查询操作 select
    MYSQL_RES* query(string sql);

    // 刷新一下连接空闲时间的起始点
    void refreshAliveTime();

    // 获取连接的入队空闲时间
    clock_t getAliveTime();

    // 获取连接，用于主键回填
    MYSQL* getConnection();

private:
    MYSQL *_conn;           // 表示和MySQL Server的一条连接
    clock_t _aliveTime;     // 表示进入队列之后的连接的存活时间
};


