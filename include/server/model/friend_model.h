#ifndef FRIEND_MODEL_H
#define FRIEND_MODEL_H

#include <vector>

#include "db_connection_pool.h"
#include "user.hpp"

// 维护好友信息的操作接口方法
class FriendModel{
public:
    // 添加好友关系
    void insert(int userid, int friendid);

    // 返回用户的好友列表
    vector<User> query(int userid);
};

#endif