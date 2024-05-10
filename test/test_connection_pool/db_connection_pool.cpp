#include "db_connection_pool.h"


// 2. 获取连接池单例对象的接口函数（线程安全的懒汉单例模式，即，第一次获取实例的时候才进行实例化）
DbConnectionPool* DbConnectionPool::getDbConnectionPoolInstance(){
    static DbConnectionPool pool;   // 静态变量由编译器进行lock、unlock操作，所以是线程安全的
    return &pool;
}


// 加载配置文件
bool DbConnectionPool::loadConfigFile(){
    FILE* pf = fopen("db.cnf", "r");
    if(pf == nullptr){
        LOG("db.cnf file is not exist!");
        return false;
    }

    while(!feof(pf)){   // feof 函数判断文件是否结束，根据符号0判断
        char line[1024] = {0};
        fgets(line, 1024, pf);  // 从文件中读取一行文本
        string str = line;
        
        int idx = str.find("=", 0);
        if(idx == -1){          // 无效配置项
            continue;
        }

        // password=123456\n
        int endidx = str.find("\n", idx);
        string key = str.substr(0, idx);                        // idx个字符
        string value = str.substr(idx + 1, endidx - 1 - idx);   // endidx - 1 - idx个字符

        if(key == "ip"){
            _ip = value;
        }else if(key == "port"){
            _port = atoi(value.c_str());
        }else if(key == "username"){
            _user = value;
        }else if(key == "password"){
            _password = value;
        }else if(key == "initSize"){
            _initSize = atoi(value.c_str());
        }else if(key == "maxSize"){
            _maxSize = atoi(value.c_str());
        }else if(key == "maxIdleTime"){
            _maxIdleTime = atoi(value.c_str());
        }else if(key == "connectionTimeout"){
            _connectionTimeout = atoi(value.c_str());
        }else if(key == "dbname"){
            _dbname = value;
        }else{
            LOG("invalid key\n");
        }
    }

    return true;
}


// 构造函数
DbConnectionPool::DbConnectionPool(){
    // 加载配置文件
    if(!loadConfigFile())
        return;

    // 创建初始数量的连接，将连接放入队列中
    for(int i = 0; i < _initSize; i++){
        MySQLConnection *p = new MySQLConnection();
        p->connect(_ip, _port, _user, _password, _dbname);
        p->refreshAliveTime();
        _connectionQueue.push(p);
        _connectionCnt++;
    }

    // 启动一个新的线程，作为连接生产者线程 linux下 thread 底层调用的是 pthread_create
    thread produce(std::bind(&DbConnectionPool::produceConnectionTask, this));          // 使用thread类实例化一个对象 produce（绑定回调函数，并启动线程）
    produce.detach();   // 守护线程（若设置为守护线程，主线程退出时，由于守护线程还在继续运行，故该进程不会自动结束）

    // 启动一个新的定时线程，扫描超过maxIdleTime时间的空闲连接，进行多余的连接回收
    thread scanner(std::bind(&DbConnectionPool::scannerConnectionTask, this));
    scanner.detach();   // 守护线程
}


// 生产者线程，专门负责生产新连接
void DbConnectionPool::produceConnectionTask(){
    for(;;){
        unique_lock<mutex> lock(_queueMutex);
        while(!_connectionQueue.empty()){
            _cv.wait(lock);     // 如果队列不空，该生产线程进入阻塞状态，并且释放锁（释放锁，让消费者线程可以获取到锁）；当_cv被设置唤醒，则重新获取锁。
        }

        // 如果队列为空，则生产连接，并放入队列中
        if(_connectionCnt < _maxSize){
            MySQLConnection *p = new MySQLConnection();
            p->connect(_ip, _port, _user, _password, _dbname);
            p->refreshAliveTime();
            _connectionQueue.push(p);
            _connectionCnt++;
        }

        // 通知消费者线程
        _cv.notify_all();
    }
}


// 消费者，从连接池中获取一个可用的空闲连接（通过智能指针，管理指针在离开作用域后的自动归还到连接池）
shared_ptr<MySQLConnection> DbConnectionPool::getMySQLConnection(){
    unique_lock<mutex> lock(_queueMutex);

    // 这样写存在问题。即，消费者线程被唤醒的，但是重新获取锁之前，队列中的元素被其他线程抢走了，导致队列为空。
    // if(_connectionQueue.empty()){   
    //     _cv.wait_for(lock, std::chrono::seconds(_connectionTimeout));    // 队列为空，则阻塞，且释放锁；直到被唤醒或者超过设定时间自动唤醒，重新获取锁 
    // }
    // if(_connectionQueue.empty()){                                        // 如果是超出了等待时间被自动唤醒的    
    //     LOG("Get idle connection timed out...");
    //     return nullptr;
    // }

    if(_connectionQueue.empty()){   
        if(cv_status::timeout == _cv.wait_for(lock, std::chrono::seconds(_connectionTimeout))){     // 队列为空，则阻塞，且释放锁；直到被唤醒或者超过设定时间自动唤醒，重新获取锁
            if(_connectionQueue.empty()){
                LOG("Get idle connection timed out...");                                            // 如果是超出了等待时间被自动唤醒的    
                return nullptr;
            }
        };       
    }

    // 如果是被唤醒的，说明队列中已有空闲连接可以消费
    shared_ptr<MySQLConnection> sp( _connectionQueue.front(), [&](MySQLConnection *pconn){      // share_ptr指针默认的析构方法会将资源close，然而我们希望将其放回连接池中
        unique_lock<mutex> lock(_queueMutex);   // 操作队列，必须注意线程安全！
        pconn->refreshAliveTime();
        _connectionQueue.push(pconn);
    });
    _connectionQueue.pop();

    // if(_connectionQueue.empty()){
    //     _cv.notify_all();       // 谁消费了队列中的最后一个connection，谁负责通知一下生产者生产连接。
    // }
    _cv.notify_all();              // 通知生产者线程，检查一下队列是否为空，如果队列为空则开始生产连接

    return sp;
}          


// 定时线程，扫描超过maxIdleTime时间的空闲连接，进行多余的连接回收
void DbConnectionPool::scannerConnectionTask(){
    for(;;){
        this_thread::sleep_for(std::chrono::seconds(_maxIdleTime));     // 休眠maxIdleTime时间
        
        // 扫描整个队列，释放多余的连接
        unique_lock<mutex> lock(_queueMutex);
        while(_connectionCnt > _initSize){
            MySQLConnection *p = _connectionQueue.front();      // 队首的存活时间最长
            if(p->getAliveTime() >= (_maxIdleTime * 1000)){     // 
                _connectionQueue.pop();
                _connectionCnt--;
                delete p;   // 调用~MySQLConnection()释放连接
            }else{
                break;      // 队头的连接没有超过，其他连接肯定没有
            }
        }
    }
} 
