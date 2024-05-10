#include "chat_server.h"
#include "json.hpp"
#include "chat_service.h"

ChatServer::ChatServer(muduo::net::EventLoop *loop,
            const muduo::net::InetAddress &listenAddr,
            const string &nameArg)
    : _server(loop, listenAddr, nameArg), _loop(loop)
{
    // 注册连接回调
    _server.setConnectionCallback(std::bind(&ChatServer::onConnectionCallback, this, _1));

    // 注册消息回调
    _server.setMessageCallback(std::bind(&ChatServer::onMessageCallback, this, _1, _2, _3));
 
    // 设置线程数量
    _server.setThreadNum(4);
};

ChatServer::~ChatServer(){

}

// 启动服务
void ChatServer::start(){
    // 启动服务器
    _server.start();
}

// 用户连接断开事件发生时的 回调函数
void ChatServer::onConnectionCallback(const muduo::net::TcpConnectionPtr& conn){
    // 客户端断开连接
    if(!conn->connected()){
        ChatService::instance()->clientCloseException(conn);
        conn->shutdown();
    }

}

// 新用户的消息事件时的 回调函数。该处代码不需要改动，目的是降低网络模块和业务模块的耦合度
void ChatServer::onMessageCallback(const muduo::net::TcpConnectionPtr& conn,
                        muduo::net::Buffer* buffer,
                        muduo::Timestamp time){
    string buf = buffer->retrieveAllAsString();
    // 反序列化
    json js = json::parse(buf); 

    // 目的：解耦网络模块和业务模块
    // 通过 js["msgid"] 获取数据，回调 业务handler 进行处理
    auto msgHandler = ChatService::instance()->getHandler(js["msgid"].get<int>());
    
    // 回调消息绑定号的事件处理函数，来执行相应的业务处理
    msgHandler(conn, js, time);
}
