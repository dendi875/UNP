/**
 * 在连接建立前后显示套接字选项接收缓冲区大小和MSS值
 */

#include "unp.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

int main(int argc, char **argv)
{
    int     sockfd;
    struct  sockaddr_in     serveraddr;
    int                     rcvbuf, mss;
    socklen_t               len;

    if (argc != 2) {
        err_quit("usage：./rcvbuf <IPaddress>");
    }

    /* 创建一个 IPv4的字节流类型的TCP套接字 */
    sockfd = Socket(AF_INET, SOCK_STREAM, 0);

    /**
     * 连接建立之前打印套接字选项接收缓冲区大小和MSS值
     */
    len = sizeof(rcvbuf);
    Getsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &rcvbuf, &len);
    len = sizeof(mss);
    Getsockopt(sockfd, IPPROTO_TCP, TCP_MAXSEG, &mss, &len);
    printf("defaults：SO_RCVBUF = %d, mss = %d\n", rcvbuf, mss);

    /* 把 IPv4套接字地址结构体变量初始化为0 */
    memset(&serveraddr, 0, sizeof(serveraddr));

    /* 填充结构体 */
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(13);    /* daytime server */
    Inet_pton(AF_INET, argv[1], &serveraddr.sin_addr);

    /* 与服务器建立TCP连接 */
    Connect(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));

    /**
     * 连接建立之后打印套接字选项接收缓冲区大小和MSS值
     */
    len = sizeof(rcvbuf);
    Getsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &rcvbuf, &len);
    len = sizeof(mss);
    Getsockopt(sockfd, IPPROTO_TCP, TCP_MAXSEG, &mss, &len);
    printf("after connect：SO_RCVBUF = %d, mss = %d\n", rcvbuf, mss);

    exit(0);
}

/*
实验：
[dendi875@localhost Chapter-7 Socket Options]$ ./rcvbuf 127.0.0.1
defaults：SO_RCVBUF = 87380, mss = 536
after connect：SO_RCVBUF = 87380, mss = 16384

[dendi875@localhost Chapter-7 Socket Options]$ ./rcvbuf 192.168.100.130
defaults：SO_RCVBUF = 87380, mss = 536
after connect：SO_RCVBUF = 87380, mss = 1448
*/