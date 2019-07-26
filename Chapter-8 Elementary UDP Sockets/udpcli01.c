/**
 * UDP 回射程序－客户端
 */

#include "unp.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main(int argc, char **argv)
{
    int     sockfd;
    struct  sockaddr_in     serveraddr;

    if (argc != 2) {
        err_quit("usage：./udpcli01 <IPaddress>");
    }

    /* 把 IPv4套接字地址结构体变量初始化为0 */
    memset(&serveraddr, 0, sizeof(serveraddr));

    /* 填充结构体 */
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(SERV_PORT);
    Inet_pton(AF_INET, argv[1], &serveraddr.sin_addr);

    /* 创建一个 IPv4套接字类型为数据报的UDP套接字 */
    sockfd = Socket(AF_INET, SOCK_DGRAM, 0);

    dg_cli(stdin, sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));

    exit(0);
}