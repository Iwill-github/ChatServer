// 数据库增删改查相关

#include "db.hpp"

// 初始化数据库连接
MySQLConnection::MySQLConnection(){
    _conn = mysql_init(nullptr);
} 


// 释放数据库连接资源
MySQLConnection::~MySQLConnection(){
    if (_conn != nullptr)   
        mysql_close(_conn);
} 


// 连接数据库
bool MySQLConnection::connect(string ip, unsigned short port, 
                        string user, string password, string dbname){
    MYSQL *p = mysql_real_connect(_conn, ip.c_str(), user.c_str(),
    password.c_str(), dbname.c_str(), port, nullptr, 0);
    return p != nullptr;
} 


// 更新操作 insert、delete、update
bool MySQLConnection::update(string sql){
    if (mysql_query(_conn, sql.c_str())){
        LOG("更新失败:" + sql);
        return false;
    } 
    
    LOG("更新成功:" + sql);
    return true;
} 


// 查询操作 select
MYSQL_RES* MySQLConnection::query(string sql){
    if (mysql_query(_conn, sql.c_str())){
        LOG("查询失败:" + sql);
        return nullptr;
    } 
    return mysql_use_result(_conn);
}


// 刷新一下连接空闲时间的起始点
void MySQLConnection::refreshAliveTime(){
    _aliveTime = clock();
}


// 获取连接
clock_t MySQLConnection::getAliveTime(){
    return clock() - _aliveTime;
}
