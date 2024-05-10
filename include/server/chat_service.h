#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include <muduo/net/TcpConnection.h>
#include <muduo/base/Timestamp.h>
#include <unordered_map>
#include <mutex>

#include "json.hpp"
using json = nlohmann::json;
using namespace std::placeholders;

#include "user.hpp"
#include "user_model.h"
#include "offline_message_model.h"
#include "friend_model.h"
#include "group_model.h"
#include "redis.h"


// 表示处理消息的事件回调类型
using MsgHandler = std::function<void(const muduo::net::TcpConnectionPtr&,
                                      nlohmann::json&,
                                      muduo::Timestamp)>;

// 聊天服务器业务类。单例模式。service层
class ChatService{
public:
    static ChatService* instance();

    // 业务函数
    void login(const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp time);          // 处理登录业务
    void reg(const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp time);            // 处理注册业务
    void loginout(const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp time);       // 处理注销业务
    void oneChat(const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp time);        // 一对一聊天业务
    void addFriend(const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp time);      // 添加好友业务

    void createGroup(const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp time);    // 创建群组业务
    void addGroup(const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp time);       // 加入群组业务
    void groupChat(const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp time);      // 群组聊天业务

    // 从redis消息队列中获取订阅的消息
    void handleRedisSubscribeMessage(int userid, string msg);

    // 处理客户端异常退出
    void clientCloseException(const muduo::net::TcpConnectionPtr& conn);

    // 服务器异常，重置方法
    void reset();

    // 获取消息对应的处理方法
    MsgHandler getHandler(int msgid);

private:
    ChatService();  // 私有化构造函数

    // 消息id和其对应的业务处理方法 的映射表。(一个线程一个业务，不需要考虑线程安全)
    std::unordered_map<int, MsgHandler> _msgHandlerMap;

    // 存储在线用户的通信连接。（多个线程同时操作时，需要考虑线程安全）
    std::unordered_map<int, muduo::net::TcpConnectionPtr> _userConnMap;

    // 定义互斥锁，保证_userConnMap的线程安全
    mutex _connMutex;

    // 数据操作类对象
    UserModel _userModel;
    OfflineMessageModel _offlineMsgModel;
    FriendModel _friendModel;
    GroupModel _groupModel;

    // redis操作对象
    Redis _redis;
};

#endif
