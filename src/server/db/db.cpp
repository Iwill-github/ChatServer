#include "db.hpp"

// 初始化数据库连接
MySQL::MySQL(){
    _conn = mysql_init(nullptr);
}

MySQL::~MySQL(){
    if (_conn != nullptr)
        mysql_close(_conn);
} 

// 连接数据库
bool MySQL::connect(){
    MYSQL *p = mysql_real_connect(_conn, server.c_str(), user.c_str(), password.c_str(), dbname.c_str(), 3306, nullptr, 0);
    if (p != nullptr){
        // 设置编码格式为gbk。C/C++代码默认的编码字符是ASCII，不支持中文解码
        mysql_query(_conn, "set names gbk");    
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":" << "数据库连接成功!"; 
    } else{
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":" << "数据库连接失败!";
    }
    return p;
} 

// 更新操作
bool MySQL::update(std::string sql){
    // 尝试执行SQL更新语句，如果执行失败则记录日志并返回false
    if (mysql_query(_conn, sql.c_str())){
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":" << sql << "更新失败!";
        return false;
    } 

    return true;
}

// 查询操作
MYSQL_RES* MySQL::query(std::string sql){
    if (mysql_query(_conn, sql.c_str())){
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":" << sql << "查询失败!";
        return nullptr;
    } 
    return mysql_use_result(_conn);
}

// 获取连接
MYSQL* MySQL::getConnection(){
    return _conn;
}
