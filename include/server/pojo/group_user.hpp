#ifndef GROUP_USER_H
#define GROUP_USER_H

#include "user.hpp"

// 群组用户，多了一个role角色信息，从User类直接继承，复用User的其它信息
class GroupUser : public User
{
public:
    void setRole(string role) { this->role = role; }
    string getRole() { return this->role; }

private:
    string role;    // 在群组中用户的角色信息
};

#endif