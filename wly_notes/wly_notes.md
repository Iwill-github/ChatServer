
# 常用命令
    netstat -anp | grep redis
        -a：显示所有连接和监听端口，包括正在使用的和未使用的端口。
        -n：使用数字格式显示地址和端口号，而不进行反向域名解析。
        -p：显示与每个连接或监听端口相关的进程的PID和进程名称。
    telnet <hostname or IP> <port>


# gdb 调试
    ps -u                       # 查看进程
    gdb attach pid              # 挂载进程
    infor threads               # 查看线程
    thread 2                    # 切换线程
    bt                          # 查看调用栈


    gdb ChatServer              # 进入gdb调试
    break chatservice.cpp:41    # 设置断点
    run                         # 运行程序（如果有参数，需要在run后加上，如 run 127.0.0.1 6000）
    

# unbuntu 重新装mysql步骤
    sudo apt-get update
    sudo apt purge mysql-*
    sudo rm -rf /etc/mysql/ /var/lib/mysql
    sudo apt autoremove
    sudo apt clean
    sudo apt-get install mysql-server
    sudo apt install mysql-client
    sudo apt install libmysqlclient-dev
    sudo vim /etc/mysql/mysql.conf.d/mysqld.cnf     注意：将bind-address = 127.0.0.1  改成 bind-address = 0.0.0.0
    sudo mysql -uroot -proot
    ALTER USER 'root'@'localhost' IDENTIFIED WITH mysql_native_password BY 'password123';
    CREATE USER 'root'@'%' IDENTIFIED BY 'password123';
    GRANT ALL PRIVILEGES ON *.* TO 'root'@'%' WITH GRANT OPTION;
    exit
    sudo /etc/init.d/mysql stop	    (关闭数据库)
    sudo /etc/init.d/mysql start    (启动数据库)


# mysql grant
    ## 给予wly 操作chat数据库的所有权限，并且wly可以从任何地址登录
    GRANT ALL PRIVILEGES ON chat.* TO 'wly'@'%';

    ## 设置root用户可以从任何地址登录的
    mysql -uroot -p                                # root用户登录msyql   
    use mysql;                                     # 使用mysql数据库
    show tables;                                   # 查看mysql数据库的数据表
    select host from user;                         # 查看user表的host
    update user set host ='%' where user ='root';  # 修改root用户访问权限，可以从网络访问
    flush privileges;              # 刷新用户权限（本质上是将用户信息和权限信息提取到内存中）


# mysql 备份和恢复数据库
    mysqldump -u root -p chat > chat_backup.sql
    mysql -u root -p chat < chat_backup.sql


# nginx 配置tcp负载均衡（有个非常重要的算法：一致性哈希算法，一般用于短连接服务器，chatserver是长连接服务器）
    sudo apt install nginx              # 默认配置文件目录/etc/nginx。/etc通常用于存放linux系统的配置文件。
    sudo vim /etc/nginx/nginx.conf      # 添加tcp长连接负载均衡配置（与http负载均衡配置同级）
    #########################################################################
    # nginx tcp loadbalance config
    stream{
            upstream MyServer{
                    server 127.0.0.1:6000 weight=1 max_fails=3 fail_timeout=30s;    # chatServer1 运行在本地的6000端口
                    server 127.0.0.1:6001 weight=1 max_fails=3 fail_timeout=30s;    # chatServer2 运行在本地的6002端口
            }

            server{
                    proxy_connect_timeout 1s;   
                    #proxy_timeout 3s;
                    listen 8000;                # nginx 监听的端口
                    proxy_pass MyServer;        # 向MyServer负载均衡模块进行负载均衡
                    tcp_nodelay on;
            }
    }
    #########################################################################
    sudo nginx -s reload                        # 平滑加载nginx配置文件
    sudo nginx                                  # 启动nginx
    netstat -anp | grep 8000                    # 查看监听8000端口的进程
    nginx -s stop                               # 停止nginx服务


# redis 发布-订阅相关命令（redis本质上是一个存储键值对的缓存数据库）
    # redis安装
    sudo apt-get install redis-server           # 安装redis（安装完成后，自动启动）
    netstat -anp | grep redis                   # 查看redis进程
    
    # 初识redis命令
    redis-cli                   # 登录redis
    set "abc" "hello world"     # 设置键值对
    get "agc"                   # 获取键值对
    subscribe 13                # 订阅13通道消息（一般订阅这个通道id取用户id）
    publish 13 "hello world"    # 发布13通道消息
    
    # 安装redis客户端库
    # redis支持多种不同的客户端编程语言，Java对应jedis，Python对应redis-py，C++对应hiredis，Go对应redigo，PHP对应phpredis等。
    sudo git clone https://github.com/redis/hiredis     # 下载hiredis（C++语言写的redis客户端库）
    cd hiredis
    sudo make                                           # 编译hiredis
    sudo make install                                   # 安装hiredis
    sudo ldconfig /usr/local/lib                        # 刷新动态链接库


# 测试
    # 注意负载均衡器是工作在8000端口的，所以要使用8000端口进行测试
    ./ChatServer 127.0.0.1 6000
    ./ChatServer 127.0.0.1 6001
    ./ChatClient 127.0.0.1 8000


