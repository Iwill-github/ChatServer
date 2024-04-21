#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <string>

using namespace std;

// 聊天服务器类
class ChatServer
{
public:
    // 有参构造。初始化聊天服务器对象
    ChatServer(muduo::net::EventLoop *loop,
               const muduo::net::InetAddress &listenAddr,
               const string &nameArg);
    
    ~ChatServer();
    
    // 启动服务
    void start();

    // 用户连接断开事件发生时的 回调函数
    void onConnectionCallback(const muduo::net::TcpConnectionPtr&);

    // 新用户的消息事件时的 回调函数
    void onMessageCallback(const muduo::net::TcpConnectionPtr&,
                           muduo::net::Buffer*,
                           muduo::Timestamp);

private:
    muduo::net::TcpServer _server;  // 组合muduo库，实现服务器功能的类
    muduo::net::EventLoop *_loop;   // 指向事件循环对象的指针
};

#endif