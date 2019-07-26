/**
 * 基于 TCP 的回射程序－客户端（使用 select 多路复用）
 * NOTE；服务端是 udpservselect01.c
 */

#include "unp.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

void str_cli_select(FILE *fp, int sockfd)
{
    int     maxfdp1;
    fd_set  rset;
    char    sendline[MAXLINE], recvline[MAXLINE];
    int     n;

    /* 初始化套接字文件描述符集合 */
    FD_ZERO(&rset);
    for (;;) {
        FD_SET(fileno(fp), &rset);
        FD_SET(sockfd, &rset);
        maxfdp1 = max(fileno(fp), sockfd) + 1;
        Select(maxfdp1, &rset, NULL, NULL, NULL);

        if (FD_ISSET(fileno(fp), &rset)) {      /* 标准输入可读　*/
            if (Fgets(sendline, MAXLINE, fp) == NULL) {
                return;
            }
            Writen(sockfd, sendline, strlen(sendline));
        }

        if (FD_ISSET(sockfd, &rset)) {  /* 套接字可读 */
            if (Readline(sockfd, recvline, MAXLINE) == 0) {
                err_sys("str_cli: server terminated prematurely");
            }
            Fputs(recvline, stdout);
        }
    }
}

int main(int argc, char **argv)
{
    int                     sockfd;
    struct  sockaddr_in     serveraddr;

    if (argc != 2) {
        err_quit("usage：./tcpcliselect01 <IPaddress>");
    }

    /* 创建一个IPv4的套接字类型为字节流类型的TCP套接字 */
    sockfd = Socket(AF_INET, SOCK_STREAM, 0);

    /* 把IPv4套接字地址结构体变量初始化为0 */
    memset(&serveraddr, 0, sizeof(serveraddr));

    /* 填充结构体 */
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(SERV_PORT);
    Inet_pton(AF_INET, argv[1], &serveraddr.sin_addr);

    /* 连接到TCP服务器 */
    Connect(sockfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr));

    str_cli_select(stdin, sockfd);

    exit(0);
}