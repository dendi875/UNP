/**
 * UDP 回射程序－客户端。
 */

#include "unp.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define     NDG   2000   /* 要发送的数据报 */
#define     DGLEN 1400   /* 每个数据报的长度 */

void dg_cli_loop2(FILE *fp, int sockfd, const struct sockaddr *pservaddr, socklen_t servlen)
{
    int     i;
    char    sendline[DGLEN];

    for (i = 0; i < NDG; i++) {
        Sendto(sockfd, sendline, DGLEN, 0, pservaddr, servlen);
    }
}

int main(int argc, char **argv)
{
    int    sockfd;
    struct sockaddr_in  serveraddr;

    if (argc != 2) {
        err_quit("./udpcli04 <IPaddress>");
    }

    /* 创建一个IPv4的数据报的UDP套接字 */
    sockfd = Socket(AF_INET, SOCK_DGRAM, 0);

    /* 初始化IPv4的套接字地址结构体变量为0 */
    memset(&serveraddr, 0, sizeof(serveraddr));

    /* 填充结构体 */
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(SERV_PORT);
    Inet_pton(AF_INET, argv[1], &serveraddr.sin_addr);

    dg_cli_loop2(stdin, sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));

    exit(0);
}