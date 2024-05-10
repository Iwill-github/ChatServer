#include <iostream> 
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

int main(int argc, char *argv[]) {
    unlink("sock.s");   // 删除socket文件

    // 创建unix流式套接字
    int lfd = socket(AF_UNIX, SOCK_STREAM, 0);  // SOCK_STREAM表示tcp连接类型
    
    // 绑定
    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, "sock.s");
    socklen_t len = offsetof(struct sockaddr_un, sun_path) + strlen(addr.sun_path);   // sun_path成员在sockaddr_un结构体中的偏移量 + sun_path的长度
    bind(lfd, (struct sockaddr *)&addr, len);          // (struct sockaddr *) 是将地址转换为通用地址结构体指针

    // 监听
    // 当服务器调用 listen(sockfd, backlog)后，操作系统就会为该套接字创建一个连接请求队列，并开始监听该队列。
    // backlog参数指定了这个队列的大小。如果有新的连接请求到达，而当前没有超过队列的最大长度，请求就会被放入队列中等待处理；
    // 如果队列已满，则根据系统设置的不同，可能拒绝新的连接请求或者让请求等待更长时间。
    listen(lfd, 5);

    // 提取
    struct sockaddr_un cliaddr;
    socklen_t len_c = sizeof(cliaddr);
    int cfd = accept(lfd, (struct sockaddr *)&cliaddr, &len_c);
    printf("new client file = %s\n", cliaddr.sun_path);
    
    // 读写
    char buf[1024] = "";
    while(1){
        int n = recv(cfd, buf, sizeof(buf), 0);
        if(n <= 0){
            printf("err or client close\n");
            break;
        }else{
            printf("%s", buf);
            send(cfd, buf, n, 0);
        }
    }

    // 关闭
    close(lfd);
    close(cfd);

    return 0;
}

