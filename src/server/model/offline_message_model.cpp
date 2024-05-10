#include "offline_message_model.h"
#include "db.h"

// 存储用户的离线消息
void OfflineMessageModel::insert(int userid, std::string msg){
    // 组装sql语句
    char sql[1024] = {0};
    // .c_str() 是 std::string 类的一个成员函数，用于获取该字符串对象内部存储的 C 风格字符串（即以 \0 结尾的字符数组）。返回值类型为 const char*。
    // snprintf 能有效防止因目标缓冲区过小而导致的缓冲区溢出，提高了代码的安全性。
    snprintf(sql, sizeof(sql), "insert into offlinemessage values('%d', '%s');", 
                userid, msg.c_str());

    // MySQL mysql;
    // if(mysql.connect()){
    //     mysql.update(sql);
    // }

    DbConnectionPool *cp = DbConnectionPool::getDbConnectionPoolInstance();
    shared_ptr<MySQLConnection> sp = cp->getMySQLConnection(); 
    if(sp != nullptr){
        sp->update(sql);
    }
}

// 删除用户的离线消息
void OfflineMessageModel::remove(int userid){
    // 组装sql语句
    char sql[1024] = {0};
    // .c_str() 是 std::string 类的一个成员函数，用于获取该字符串对象内部存储的 C 风格字符串（即以 \0 结尾的字符数组）。返回值类型为 const char*。
    // snprintf 能有效防止因目标缓冲区过小而导致的缓冲区溢出，提高了代码的安全性。
    snprintf(sql, sizeof(sql), "delete from offlinemessage where userid = %d;", userid);

    // MySQL mysql;
    // if(mysql.connect()){
    //     mysql.update(sql);
    // }

    DbConnectionPool *cp = DbConnectionPool::getDbConnectionPoolInstance();
    shared_ptr<MySQLConnection> sp = cp->getMySQLConnection();
    if(sp != nullptr){
        sp->update(sql);
    }
}

// 查询用户的离线消息
std::vector<std::string> OfflineMessageModel::query(int userid){
    // 组装sql语句
    char sql[1024] = {0};
    snprintf(sql, sizeof(sql), "select message from offlinemessage where userid = %d;", userid);

    // std::vector<std::string> vec;
    // MySQL mysql;
    // if(mysql.connect()){    // 连接数据库
    //     if(MYSQL_RES* res = mysql.query(sql)){
    //         // 查询成功，封装对象后返回
    //         MYSQL_ROW row;
    //         while (row = mysql_fetch_row(res)){ 
    //             vec.emplace_back(row[0]);
    //         }

    //         mysql_free_result(res);
    //         return vec;
    //     }
    // }

    std::vector<std::string> vec;
    DbConnectionPool *cp = DbConnectionPool::getDbConnectionPoolInstance();
    shared_ptr<MySQLConnection> sp = cp->getMySQLConnection();

    if(sp != nullptr){    // 连接数据库
        if(MYSQL_RES* res = sp->query(sql)){
            // 查询成功，封装对象后返回
            MYSQL_ROW row;
            while (row = mysql_fetch_row(res)){ 
                vec.emplace_back(row[0]);
            }

            mysql_free_result(res);
            return vec;
        }
    }

    return vec;
}


