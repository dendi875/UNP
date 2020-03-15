/**
 * 基于 TCP 的回射程序－客户端  UNP 书上 113 例子
 */

#include "unp.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>

int main(int argc, char **argv)
{
    int                 sockfd;
    struct sockaddr_in  serveraddr;

    if (argc != 2) {
        err_quit("usage：./my_tcp_cli_terminated <IPAddress>");
    }

    /* 创建一个 IPv4 的 TCP 套接字 */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        err_sys("socker error");
    }

    /* 把 IPv4 套接字地址结构体初始化为0 */
    memset(&serveraddr, 0, sizeof(serveraddr));

    /* 填充结构体 */
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(SERV_PORT);
    if (inet_pton(AF_INET, argv[1], &serveraddr.sin_addr) <= 0) {
        err_quit("inet_pton error for %s", argv[1]);
    }

    /* 与 TCP 服务器建立连接 */
    if (connect(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) {
        err_sys("connect error");
    }

    str_cli(stdin, sockfd);

    exit(0);
}