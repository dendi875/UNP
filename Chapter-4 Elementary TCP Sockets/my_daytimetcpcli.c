/**
 * 一个简单的使用 TCP 获取时间的客户端程序
 *
 * 使用 getsockname 获取内核赋于TCP套接字的本地IP地址和本地端口
 */

#include "unp.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main(int argc, char **argv)
{
    int    sockfd, n;
    struct sockaddr_in  serveraddr, clientaddr;
    socklen_t   len;
    char        str[128];
    char        portstr[8];
    char        recviveline[MAXLINE + 1];

    if (argc != 2) {
        err_quit("uusage：./my_daytimetcpcli  <IPaddress>");
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        err_sys("socket error");
    }

    /* 把 IPv4 套接字地址结构体变量初始化为0 */
    memset(&serveraddr, 0, sizeof(serveraddr));

    /* 填充结构体 */
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(13);

    /* 因为是字符串的 IP地址 所以使用 inet_pton 来转换成网络字节序 */
    if (inet_pton(AF_INET, argv[1], &serveraddr.sin_addr) <= 0) {
        err_quit("inet_pton error for %s", argv[1]);
    }

    /* 与服务器建立一个TCP连接 */
    if (connect(sockfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) {
        err_sys("connect error");
    }

    len = sizeof(clientaddr);
    if (getsockname(sockfd, (struct sockaddr *) &clientaddr, &len) < 0) {
        err_sys("getsockname error");
    }

    /**
     * 把结构体中的二进制形式的网络字节序 IP和端口 转成字符串IP和整数端口，
     * 然后再使用 snprintf 把整数端口转成字符串形式
     */
    if (inet_ntop(AF_INET, &clientaddr.sin_addr, str, sizeof(str)) == NULL) {
        err_sys("inet_ntop error");
    }

    snprintf(portstr, sizeof(portstr), ":%d", ntohs(clientaddr.sin_port));
    strcat(str, portstr);

    printf("local addr：%s\n", str);

    /**
     * 使用 read 读取服务器的应答，并调用 fputs 输出结果
     */
    while ((n = read(sockfd, recviveline, MAXLINE)) > 0) {
        recviveline[n] = 0;     /* 添加空字符，代表字符串的终止 */
        if (fputs(recviveline, stdout) == EOF) {
            err_sys("fputs error");
        }
    }

    if (n < 0) {
        err_sys("read error");
    }

    exit(0);
}

/*
实验：
[dendi875@localhost Chapter-4 Elementary TCP Sockets]$ ./my_daytimetcpcli 192.168.100.142
local addr：192.168.100.142:58641
08 JUN 2019 16:02:05 CST

[dendi875@localhost Chapter-4 Elementary TCP Sockets]$ ./my_daytimetcpcli 192.168.100.142
local addr：192.168.100.142:58642
08 JUN 2019 16:02:16 CST
*/