#include <iostream> 
#include <unistd.h>

#include <sys/socket.h>
#include <sys/un.h>

int main(int argc, char *argv[]) {
    unlink("sock.c");

    // 创建unix流式套接字
    int cfd = socket(AF_UNIX, SOCK_STREAM, 0);  // SOCK_STREAM表示tcp连接类型
    
    // 绑定（如果不绑定，就是隐式绑定）
    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, "sock.c");
    socklen_t len = offsetof(struct sockaddr_un, sun_path) + strlen(addr.sun_path);   // sun_path成员在sockaddr_un结构体中的偏移量 + sun_path的长度
    if(bind(cfd, (struct sockaddr *)&addr, len) < 0){                                 // (struct sockaddr *) 是将地址转换为通用地址结构体指针
        perror("bind err\n");
        return 0;
    };          

    // 连接
    struct sockaddr_un seraddr;
    seraddr.sun_family = AF_UNIX;
    strcpy(seraddr.sun_path, "sock.s");
    connect(cfd, (struct sockaddr *)&seraddr, sizeof(seraddr));
    
    // 读写
    char buf[1024] = "";
    while(1){
        int n = read(STDIN_FILENO, buf, sizeof(buf));   // 读取标准输入
        send(cfd, buf, n, 0);
        memset(buf, 0, sizeof(buf));

        n = recv(cfd, buf, sizeof(buf), 0);
        if(n <= 0){
            printf("err or server close\n");
            break;
        }else{
            printf("%s", buf);
        }
    }

    // 关闭
    close(cfd);

    return 0;
}

