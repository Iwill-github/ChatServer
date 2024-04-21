#ifndef OFFLINEMESSAGEMODEL_HPP
#define OFFLINEMESSAGEMODEL_HPP

#include <string>
#include <vector>

class OfflineMessageModel{
public:
    // 存储用户的离线消息
    void insert(int userid, std::string msg);

    // 删除用户的离线消息
    void remove(int userid);

    // 查询用户的离线消息
    std::vector<std::string> query(int userid);
};


#endif
