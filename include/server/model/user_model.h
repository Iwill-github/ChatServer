#ifndef USERMODEL_H
#define USERMODEL_H

#include "user.hpp"

// 用户表的数据操作类。dao层
class UserModel
{
public:
    // 注册用户
    bool insert(User& user);

    // 根据用户号码查询用户信息
    User query(int id);

    // // 更新用户状态信息
    bool updateState(User& user);

    // 重置用户的状态信息
    void resetState();

private:
    
};

#endif