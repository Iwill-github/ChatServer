
/*
    muduo网络库给用户提供了两个主要的类
        TcpServer：用于编写服务器程序
        TcpClient：用于编写客户端程序

    epool + 线程池
        优点：把网络I/O和业务代码(用户连接/断开、用户的可读写事件)分开
*/
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <functional>   // 绑定器
#include <iostream>
#include <string>
using namespace std;


/*
    基于muduo网络库开发服务器程序
        1. 组合TcpServer对象
        2. 创建EventLoop事件循环对象的指针
        3. 明确TcpServer构造函数需要什么参数，以及调用TcpServer的成员函数
        4. 在当前服务器类的构造函数中，分别注册处理 连接、读写事件 的回调函数
        5. 设置合适的服务端线程数量，muduo库会自己分配io线程和worker线程
*/ 

class ChatServer
{
public:
    ChatServer(muduo::net::EventLoop *loop,                     // 事件循环
                const muduo::net::InetAddress &listenAddr,      // IP + Port
                const string &nameArg)                          // 服务器名称
        : _server(loop, listenAddr, nameArg), _loop(loop)
    {
        // 给服务器注册 用户连接的创建和断开回调（用户连接的创建和断开发生时，做什么事情）
        // 使用 std::bind 创建了该回调，它将 this 作为第一个（隐含的）参数传递给 onConnection 函数。这样做确保了在调用回调时，onConnection 能够访问到当前 ChatServer 实例的成员变量和方法。
        _server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, 
            std::placeholders::_1));  

        // 给服务器注册 用户读写事件回调（读写事件发生时，做什么事情）
        _server.setMessageCallback(std::bind(&ChatServer::onMessagetion, this, 
            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));  // 参数占位符

        // 设置服务器端的线程数量。1个io线程，3个worker线程
        _server.setThreadNum(4);
    }

    void start(){
        _server.start();
    }

private:
    // 专门处理 用户的连接创建和断开  epoll lisenfd accept
    // 该函数是一个回调函数，当TCP连接状态发生变化时会被调用。
    // 该函数的参数为一个指向TcpConnection对象的智能指针，可以用来获取连接相关信息或进行相应的处理操作。
    void onConnection(const muduo::net::TcpConnectionPtr& conn){
        cout << conn->peerAddress().toIpPort() << " -> " << conn->localAddress().toIpPort() << " is "
             << (conn->connected() ? "online" : "offline") << endl;
        if(!conn->connected())
            conn->shutdown();
    }

    void onMessagetion(const muduo::net::TcpConnectionPtr& conn,    // 连接
                       muduo::net::Buffer *buffer,                  // 缓冲区
                       muduo::Timestamp time){                           // 时间戳
        string buf = buffer->retrieveAllAsString();
        cout << "recv data: " << buf << "time:" << time.toString() <<  endl;
        conn->send(buf);
    }

    muduo::net::TcpServer _server;  // # 1
    muduo::net::EventLoop *_loop;   // # 2
};


int main(){
    muduo::net::EventLoop loop;                         // 事件循环对象
    muduo::net::InetAddress addr("127.0.0.1", 6000);    // IP + Port
    ChatServer server(&loop, addr, "WlyChatServer");    // 创建服务器对象

    server.start();     // 启动服务器，监听客户端连接。
    loop.loop();        // 启动事件循环，监听网络事件。调用了epoll_wait等待，以阻塞的模式等待某些事件发生，会调用对应事件的回调函数处理。

    return 0;
}
 

/*
客户端连接的操作是由 epoll 事件循环来完成的。具体过程如下：
    启动监听： 
        在 server.start() 中，服务器创建并配置监听套接字，然后将其注册到 epoll 实例中，关注“可接受”事件。
        此时，服务器已经准备好接收来自客户端的连接请求。
    等待事件： 
        调用 loop.loop() 启动事件循环，进入 epoll_wait 阻塞等待状态。
        epoll_wait 会一直等待，直到监听套接字或其他已连接套接字上有感兴趣的事件发生。
    接收到连接事件： 
        当有客户端尝试连接到服务器时，操作系统触发监听套接字上的“可接受”事件。
        epoll_wait 检测到这个事件后立即返回，告诉事件循环有新的连接请求到来。
    处理连接事件： 
        事件循环在收到“可接受”事件后，调用 accept 系统调用接受这个新连接。
        accept 会创建一个新的套接字来代表与客户端的连接，并返回这个新套接字的文件描述符。
    注册新连接到 epoll： 
        服务器将新创建的套接字添加到 epoll 实例中，通常关注“可读”、“可写”、错误和关闭等事件，以便后续处理该连接的读写操作和连接状态变化。
    回调通知： 
        如果服务器库提供了连接回调（如 onConnection），事件循环可能会在接收到新连接后调用该回调，通知应用程序有新客户端连接到服务器。
    继续事件循环： 
        处理完新连接事件后，事件循环重新调用 epoll_wait，继续阻塞等待下一次网络事件的发生。
*/