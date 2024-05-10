/*
    接池功能模块
*/
#pragma once
#include <iostream>
#include <string>
#include <queue>

#include <mutex>        // 保证队列操作的线程安全
#include <atomic>       // 原子操作
#include <thread>
#include <condition_variable>

#include <memory>       // 智能指针
#include <functional>

#include "db.hpp"
#include "public.h"

using namespace std;

class DbConnectionPool{
public:
    static DbConnectionPool* getDbConnectionPoolInstance();     // 2. 获取连接池单例对象的接口函数 
    shared_ptr<MySQLConnection> getMySQLConnection();           // 消费者，从连接池中获取一个可用的空闲连接（通过智能指针，管理指针在离开作用域后的自动归还到连接池）

private:
    DbConnectionPool();                       // 1. 构造函数私有化
    bool loadConfigFile();                      // 加载配置文件
    void produceConnectionTask();               // 生产者线程，专门负责生产新连接
    void scannerConnectionTask();               // 定时线程，扫描超过maxIdleTime时间的空闲连接，进行多余的连接回收

    string _ip;                                 // 数据库ip 
    unsigned short _port;                       // 数据库端口
    string _user;                               // 数据库用户名
    string _password;                           // 数据库密码
    string _dbname;                             // 数据库名称
    int _initSize;                              // 初始化连接池大小
    int _maxSize;                               // 连接池最大连接数
    int _maxIdleTime;                           // 连接池最大空闲时间
    int _connectionTimeout;                     // 连接池获取连接超时时间

    queue<MySQLConnection*> _connectionQueue;   // 连接池队列
    mutex _queueMutex;                          // 维护连接池队列线程安全的互斥锁
    atomic_int _connectionCnt;                  // 连接池中的连接数量，该数量不能超过 _maxSize
    condition_variable _cv;                     // 设置条件变量，用于 生产线程 和 消费线程 的通信
};

