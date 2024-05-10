#ifndef REDIS_H
#define REDIS_H

#include <hiredis/hiredis.h>
#include <thread>
#include <functional>
using namespace std;

/*
    redis作为集群服务器通信的基于发布-订阅消息队列时，会遇到两个难搞的bug问题，参考我的博客详细描述：
        https://blog.csdn.net/QIANGWEIYUAN/article/details/97895611
*/
class Redis
{
public:
    Redis();
    ~Redis();

    // 连接redis服务器。连接成功后生成 _publish_context 和 _subcribe_context 对象，同时开启 observer_channel_message线程 监听通道上的事件
    // 之所以开启一个线程来监听通道上的事件，是因为 hiredis 是同步阻塞的，订阅一个通道后会阻塞等待响应，导致不可以订阅第二个。
    bool connect(); 

    bool publish(int channel, string message);      // 向redis指定的通道channel发布消息
    bool subscribe(int channel);                    // 向redis指定的通道subscribe订阅消息
    bool unsubscribe(int channel);                  // 向redis指定的通道unsubscribe取消订阅消息    
    void observer_channel_message();                // 在独立线程中接收订阅通道中的消息

    // 初始化向业务层上报通道消息的回调对象
    void init_notify_handler(function<void(int, string)> fn);

private:
    redisContext *_publish_context;             // hiredis同步上下文对象，负责publish消息    
    redisContext *_subcribe_context;            // hiredis同步上下文对象，负责subscribe消息

    // 回调操作，收到订阅的消息，给service层上报
    function<void(int, string)> _notify_message_handler;
};

#endif

