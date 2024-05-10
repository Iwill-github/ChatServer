#include <user_model.h>
#include <db.h>
#include <iostream>

#include "db_connection_pool.h"

using namespace std;

// 注册用户
bool UserModel::insert(User& user){
    // 组装sql语句
    char sql[1024] = {0};
    // .c_str() 是 std::string 类的一个成员函数，用于获取该字符串对象内部存储的 C 风格字符串（即以 \0 结尾的字符数组）。返回值类型为 const char*。
    // snprintf 能有效防止因目标缓冲区过小而导致的缓冲区溢出，提高了代码的安全性。
    snprintf(sql, sizeof(sql), "insert into user(name, password) values('%s', '%s');", 
                user.getName().c_str(), user.getPassword().c_str());

    // MySQL mysql;
    // if(mysql.connect()){
    //     if(mysql.update(sql)){
    //         // 获取插入成功的用户数据生成的主键id（主键回填）
    //         user.setId(mysql_insert_id(mysql.getConnection()));
    //         return true;
    //     }
    // }

    DbConnectionPool *cp = DbConnectionPool::getDbConnectionPoolInstance();
    shared_ptr<MySQLConnection> sp = cp->getMySQLConnection();
    if(sp != nullptr){
        if(sp->update(sql)){
            // 获取插入成功的用户数据生成的主键id（主键回填）
            user.setId(mysql_insert_id(sp->getConnection()));
            return true;
        }
    }

    return false;
}


// 根据用户号码查询用户信息
User UserModel::query(int id){
    // 组装sql语句
    char sql[1024] = {0};
    snprintf(sql, sizeof(sql), "select * from user where id = %d;", id);

    // MySQL mysql;
    // if(mysql.connect()){    // 连接数据库
    //     if(MYSQL_RES* res = mysql.query(sql)){
    //         // 查询成功，封装对象后返回
    //         if(MYSQL_ROW row = mysql_fetch_row(res)){ 
    //             User user;
    //             user.setId(atoi(row[0]));
    //             user.setName(row[1]);
    //             user.setPassword(row[2]);
    //             user.setState(row[3]);

    //             mysql_free_result(res); // 释放结果集，否则内存泄漏（申请了内存未释放）
    //             return user;
    //         }
    //     }
    // }

    DbConnectionPool *cp = DbConnectionPool::getDbConnectionPoolInstance();
    shared_ptr<MySQLConnection> sp = cp->getMySQLConnection();
    if(sp != nullptr){    // 连接数据库
        if(MYSQL_RES* res = sp->query(sql)){
            // 查询成功，封装对象后返回
            if(MYSQL_ROW row = mysql_fetch_row(res)){ 
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setPassword(row[2]);
                user.setState(row[3]);

                mysql_free_result(res); // 释放结果集，否则内存泄漏（申请了内存未释放）
                return user;
            }
        }
    }

    return User();
}


// 更新用户状态信息
bool UserModel::updateState(User& user){
    // 组装sql语句
    char sql[1024] = {0};
    snprintf(sql, sizeof(sql), "update user set state = '%s' where id = %d;", 
                user.getState().c_str(), user.getId());

    // MySQL mysql;
    // if(mysql.connect()){    // 连接数据库
    //     if(mysql.update(sql)){
    //         return true;
    //     }
    // }

    DbConnectionPool *cp = DbConnectionPool::getDbConnectionPoolInstance();
    shared_ptr<MySQLConnection> sp = cp->getMySQLConnection();

    if(sp != nullptr){    // 连接数据库
        if(sp->update(sql)){
            return true;
        }
    }

    return false;
}


// 重置用户的状态信息
void UserModel::resetState(){
    // 组装sql语句
    char sql[1024] = "update user set state = 'offline' where state = 'online';";

    // MySQL mysql;
    // if(mysql.connect()){    // 连接数据库
    //     mysql.update(sql);
    // }

    DbConnectionPool *cp = DbConnectionPool::getDbConnectionPoolInstance();
    shared_ptr<MySQLConnection> sp = cp->getMySQLConnection();

    if(sp != nullptr){    // 连接数据库
        sp->update(sql);
    }
} 