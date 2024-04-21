#include "friend_model.hpp"

// 添加好友关系
void FriendModel::insert(int userid, int friendid){
    // 组装sql语句
    char sql[1024] = {0};
    // .c_str() 是 std::string 类的一个成员函数，用于获取该字符串对象内部存储的 C 风格字符串（即以 \0 结尾的字符数组）。返回值类型为 const char*。
    // snprintf 能有效防止因目标缓冲区过小而导致的缓冲区溢出，提高了代码的安全性。
    snprintf(sql, sizeof(sql), "insert into friend values(%d, %d);", 
                userid, friendid);

    MySQL mysql;
    if(mysql.connect()){
        mysql.update(sql);
    }
}


// 返回用户的好友列表
vector<User> FriendModel::query(int userid){
    // 组装sql语句
    char sql[1024] = {0};
    snprintf(sql, sizeof(sql), "select user.id, user.name, user.state from friend join user on friend.friendid = user.id where friend.userid = %d;",
                userid);

    vector<User> vec;
    MySQL mysql;
    if(mysql.connect()){    // 连接数据库
        if(MYSQL_RES* res = mysql.query(sql)){
            // 查询成功，封装对象后返回
            MYSQL_ROW row;
            while(row = mysql_fetch_row(res)){ 
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                vec.emplace_back(user);
            }
            mysql_free_result(res);
        }
    }

    return vec;
}

