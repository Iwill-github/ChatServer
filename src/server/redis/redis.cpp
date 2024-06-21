#include "redis.h"
#include <iostream>

using namespace std;


Redis::Redis(): 
    _publish_context(nullptr), _subcribe_context(nullptr)
{
}


Redis::~Redis()
{
    if (_publish_context != nullptr){
        redisFree(_publish_context);
    }

    if (_subcribe_context != nullptr){
        redisFree(_subcribe_context);
    }
}


/*
在服务器集群环境中使用 Redis 作为消息队列实现跨服务器通信时，通常会有以下几种可能的部署模式：
单一 Redis 服务器：
    最简单的情况下，集群中的所有服务器（除Redis服务器本身外）都可以连接到同一个独立的 Redis 服务器来进行消息队列通信。
    这种模式下，Redis 服务器成为系统的单点，如果它发生故障或网络中断，会影响整个集群的通信。为了确保消息可靠性，可能需要启用 Redis 的持久化功能（如 AOF 或 RDB）来定期将数据同步到磁盘，以便在 Redis 重启后能够恢复消息队列中的数据。
Redis Sentinel 高可用架构：
    为了提高 Redis 服务的可用性，可以采用 Redis Sentinel 架构。在这种情况下，有一个或多个 Sentinel 节点监控主 Redis 服务器，并在主服务器故障时自动选举新的主节点。集群中的其他服务器仍然连接到 Sentinel 系统来获取当前有效的主 Redis 地址，从而确保即使主节点发生变化，也能无缝地继续使用消息队列服务。
Redis Cluster 分布式架构：
    对于大规模、高并发或者数据量较大的场景，可能会选择 Redis Cluster 架构。Redis Cluster 将数据分布到多个节点上，每个节点管理一部分哈希槽（hash slot），形成一个分布式数据集。集群中的服务器可以连接到任意一个 Redis Cluster 节点进行消息队列操作，客户端请求会被透明地路由到正确的节点上。这样不仅提高了消息处理能力，也提供了容错性，因为即使部分节点失效，只要还有足够健康的节点以及数据冗余，集群整体依然可以继续提供服务。
*/
bool Redis::connect()
{
    // 负责publish发布消息的上下文连接
    _publish_context = redisConnect("127.0.0.1", 6379);     // 连接 redis 服务器
    if (nullptr == _publish_context)
    {
        cerr << "connect redis failed!" << endl;
        return false;
    }

    // 负责subscribe订阅消息的上下文连接
    _subcribe_context = redisConnect("127.0.0.1", 6379);    // 连接 redis 服务器
    if (nullptr == _subcribe_context)
    {
        cerr << "connect redis failed!" << endl;
        return false;
    }

    // 在单独的线程中，监听通道上的事件，有消息给业务层进行上报
    thread t([&]() {
        observer_channel_message();
    });
    t.detach(); // 守护线程。将线程与调用线程分离，使得主线程不会等待该线程结束

    cout << "connect redis-server success!" << endl;

    return true;
}


// 向redis指定的通道channel发布消息
bool Redis::publish(int channel, string message)
{
    // 这里redisCommand直接执行命令是因为 "PUBLISH %d %s" 命令不会阻塞当前线程
    redisReply *reply = (redisReply *)redisCommand(_publish_context, "PUBLISH %d %s", channel, message.c_str());
    if (nullptr == reply)
    {
        cerr << "publish command failed!" << endl;
        return false;
    }
    freeReplyObject(reply); // reply是申请在堆区的，需要手动释放
    return true;
}


// 向redis指定的通道subscribe订阅消息，只发送订阅消息，不阻塞等待消息
bool Redis::subscribe(int channel)
{
    // SUBSCRIBE命令本身会造成线程阻塞等待通道里面发生消息，这里只做订阅通道，不接收通道消息
    // 通道消息的接收专门在 observer_channel_message 函数中的独立线程中进行
    // redisCommand 命令实际上就是先 redisAppendCommand ，后redisBufferWrite，最后循环阻塞调用 redisGetReply 等待消息
    if (REDIS_ERR == redisAppendCommand(this->_subcribe_context, "SUBSCRIBE %d", channel))
    {
        cerr << "subscribe command failed!" << endl;
        return false;
    }
    // redisBufferWrite 可以循环发送缓冲区，直到缓冲区数据发送完毕（done被置为1）
    int done = 0;
    while (!done)
    {
        if (REDIS_ERR == redisBufferWrite(this->_subcribe_context, &done))
        {
            cerr << "subscribe command failed!" << endl;
            return false;
        }
    }
    // 循环阻塞调用 redisGetReply

    return true;
}


// 向redis指定的通道unsubscribe取消订阅消息
bool Redis::unsubscribe(int channel)
{
    if (REDIS_ERR == redisAppendCommand(this->_subcribe_context, "UNSUBSCRIBE %d", channel))
    {
        cerr << "unsubscribe command failed!" << endl;
        return false;
    }
    // redisBufferWrite可以循环发送缓冲区，直到缓冲区数据发送完毕（done被置为1）
    int done = 0;
    while (!done)
    {
        if (REDIS_ERR == redisBufferWrite(this->_subcribe_context, &done))
        {
            cerr << "unsubscribe command failed!" << endl;
            return false;
        }
    }

    return true;
}


// 在独立线程中接收订阅通道中的消息
void Redis::observer_channel_message()
{
    redisReply *reply = nullptr;
    while (REDIS_OK == redisGetReply(this->_subcribe_context, (void **)&reply))
    {
        // 订阅收到的消息是一个带三元素的数组
        if (reply != nullptr && reply->element[2] != nullptr && reply->element[2]->str != nullptr)
        {
            // 给业务层上报通道上发生的消息（此处为回调函数）
            _notify_message_handler(atoi(reply->element[1]->str) , reply->element[2]->str);
        }

        freeReplyObject(reply);
    }

    cerr << ">>>>>>>>>>>>> observer_channel_message quit <<<<<<<<<<<<<" << endl;
}


void Redis::init_notify_handler(function<void(int, string)> fn)
{
    this->_notify_message_handler = fn;
}