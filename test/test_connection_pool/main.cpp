#include <iostream>
#include "db.h"
#include "db_connection_pool.h"

using namespace std;


/*
    编译指令：g++ ./*.cpp -l mysqlclient -l pthread -g
*/
int main()
{
    // 不使用连接池-单线程       -- 消耗时间：11870.2 ms
    // clock_t begin = clock();
    // for(int i = 0; i < 1000; i++){
    //     MySQLConnection conn;
    //     char sql[1024] = {0};
    //     string user = "test_" + to_string(i);

    //     sprintf(sql, "insert into user(name, password, state) values('%s', '%s', '%s');",
    //         user.c_str(), "123456", "offline");
    //     conn.connect("192.168.48.129", 3306, "wly", "123456", "chat");
    //     conn.update(sql);
    // }
    // clock_t end = clock();
    // cout << "消耗时间：" << (end - begin) * 1000.0 / CLOCKS_PER_SEC << " ms" << endl; // CLOCKS_PER_SEC宏代表了每秒钟的时钟滴答数



    // 连接池-单线程            -- 消耗时间：452.033 ms
    // clock_t begin = clock();
    // DbConnectionPool *cp = DbConnectionPool::getDbConnectionPoolInstance();
    // for(int i = 0; i < 1000; i++){
    //     shared_ptr<MySQLConnection> sp = cp->getMySQLConnection();
        
    //     char sql[1024] = {0};
    //     string user = "test_" + to_string(i);
    //     sprintf(sql, "insert into user(name, password, state) values('%s', '%s', '%s');",
    //         user.c_str(), "123456", "offline");
    //     sp->update(sql);
    // }
    // clock_t end = clock();
    // cout << "消耗时间：" << (end - begin) * 1000.0 / CLOCKS_PER_SEC << " ms" << endl; // CLOCKS_PER_SEC宏代表了每秒钟的时钟滴答数



    // 连接池-多线程消耗时间：       -- 消耗时间：298.383 ms
    clock_t begin = clock();

    thread t1([](){
        DbConnectionPool *cp = DbConnectionPool::getDbConnectionPoolInstance();
        for(int i = 0; i < 250; i++){
            shared_ptr<MySQLConnection> sp = cp->getMySQLConnection();     
            char sql[1024] = {0};
            string user = "test_" + to_string(i);
            sprintf(sql, "insert into user(name, password, state) values('%s', '%s', '%s');",
                user.c_str(), "123456", "offline");
            sp->update(sql);
        }
    });
    thread t2([](){
        DbConnectionPool *cp = DbConnectionPool::getDbConnectionPoolInstance();
        for(int i = 250; i < 500; i++){
            shared_ptr<MySQLConnection> sp = cp->getMySQLConnection();     
            char sql[1024] = {0};
            string user = "test_" + to_string(i);
            sprintf(sql, "insert into user(name, password, state) values('%s', '%s', '%s');",
                user.c_str(), "123456", "offline");
            sp->update(sql);
        }
    });
    thread t3([](){
        DbConnectionPool *cp = DbConnectionPool::getDbConnectionPoolInstance();
        for(int i = 500; i < 750; i++){
            shared_ptr<MySQLConnection> sp = cp->getMySQLConnection();     
            char sql[1024] = {0};
            string user = "test_" + to_string(i);
            sprintf(sql, "insert into user(name, password, state) values('%s', '%s', '%s');",
                user.c_str(), "123456", "offline");
            sp->update(sql);
        }
    });
    thread t4([](){
        DbConnectionPool *cp = DbConnectionPool::getDbConnectionPoolInstance();
        for(int i = 750; i < 1000; i++){
            shared_ptr<MySQLConnection> sp = cp->getMySQLConnection();     
            char sql[1024] = {0};
            string user = "test_" + to_string(i);
            sprintf(sql, "insert into user(name, password, state) values('%s', '%s', '%s');",
                user.c_str(), "123456", "offline");
            sp->update(sql);
        }
    });

    t1.join();
    t2.join();
    t3.join();
    t4.join();

    clock_t end = clock();
    cout << "消耗时间：" << (end - begin) * 1000.0 / CLOCKS_PER_SEC << " ms" << endl; // CLOCKS_PER_SEC宏代表了每秒钟的时钟滴答数



#if 0
    DbConnectionPool *cp = DbConnectionPool::getDbConnectionPoolInstance();
    for(int i = 0; i < 1000; i++){
        shared_ptr<MySQLConnection> sp = cp->getMySQLConnection();
        
        char sql[1024] = {0};
        string user = "test_" + to_string(i);
        sprintf(sql, "insert into user(name, password, state) values('%s', '%s', '%s');",
            user.c_str(), "123456", "offline");
        sp->update(sql);
    }
#endif

    return 0;
 }


