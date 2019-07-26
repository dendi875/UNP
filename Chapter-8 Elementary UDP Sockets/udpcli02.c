/**
 * UDP 回射程序－客户端，使用 connect 的 dg_cli 函数
 */

#include "unp.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

void dg_cli_connect(FILE *fp, int sockfd, const struct sockaddr *pservaddr, socklen_t servlen)
{
    int     n;
    char    sendline[MAXLINE], recvline[MAXLINE + 1];

    /* 带连接的UDP */
    Connect(sockfd, pservaddr, servlen);

    while (Fgets(sendline, MAXLINE, fp) != NULL) {
        Write(sockfd, sendline, strlen(sendline));

        n = Read(sockfd, recvline, MAXLINE);

        recvline[n] = 0;    /* null terminate */
        Fputs(recvline, stdout);
    }
}


int main(int argc, char **argv)
{
    int     sockfd;
    struct  sockaddr_in    serveraddr;

    if (argc != 2) {
        err_quit("usage：./udpcli02 <IPaddress>");
    }

    /* 创建一个IPv4的数据报类型的UDP套接字 */
    sockfd = Socket(AF_INET, SOCK_DGRAM, 0);

    /* 把IPv4的套接字地址结构体初始化为0 */
    memset(&serveraddr, 0, sizeof(serveraddr));

    /* 填充结构体变量 */
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(SERV_PORT);
    Inet_pton(AF_INET, argv[1], &serveraddr.sin_addr);

    dg_cli_connect(stdin, sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));

    exit(0);
}