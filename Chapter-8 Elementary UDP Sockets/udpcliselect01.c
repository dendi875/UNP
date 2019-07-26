/**
 * UDP 回射程序－客户端
 *
 * NOTE：服务端是 udpcliselect01.c
 */

#include "unp.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main(int argc, char **argv)
{

    int     sockfd;
    struct  sockaddr_in  serveraddr;

    if (argc != 2) {
        err_quit("usage：./udpcliselect01 <IPaddress>");
    }

    /* 创建一个IPv4的套接字类型为数据报类型的UDP套接字 */
    sockfd = Socket(AF_INET, SOCK_DGRAM, 0);

    /* 把IPv4的套接字地址结构体变量初始化为0 */
    memset(&serveraddr, 0, sizeof(serveraddr));

    /* 填充结构体 */
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(SERV_PORT);
    Inet_pton(AF_INET, argv[1], &serveraddr.sin_addr);

    dg_cli(stdin, sockfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr));

    exit(0);
}

