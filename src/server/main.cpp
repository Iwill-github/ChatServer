#include "chat_server.hpp"
#include "chat_service.hpp"
#include <iostream>
#include <signal.h>

using namespace std;


// 处理服务器 ctrl + c结束后，重置user的状态信息
void resetHandler(int)
{
    ChatService::instance()->reset();
    exit(0);
};


int main(int argc, char** argv)
{
    if(argc < 3){
        cerr << "command invalid! example: ./chat_server 127.0.0.1 6000" << endl;
        exit(-1);
    }
    // 解析命令行参数获得服务端ip和端口号
    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);


    signal(SIGINT, resetHandler);   // 将SIGINT信号（中断信号）的处理函数设置为resetHandler

    muduo::net::EventLoop loop;
    muduo::net::InetAddress addr(ip, port);
    ChatServer server(&loop, addr, "ChatServer");

    server.start();
    loop.loop();

    return 0;
}
