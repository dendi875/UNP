/**
 * 基于 TCP 的客户端/服务器程序－客户端（使用 select 多路复用）
 */

#include "unp.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/select.h>

void str_cli_select(FILE *fp, int sockfd)
{
    int     maxfdp1;
    fd_set  rset;
    char    sendline[MAXLINE], recvline[MAXLINE];

    FD_ZERO(&rset);
    for (;;) {
        FD_SET(fileno(fp), &rset);
        FD_SET(sockfd, &rset);
        maxfdp1 = max(fileno(fp), sockfd) + 1;
        Select(maxfdp1, &rset, NULL, NULL, NULL);

        if (FD_ISSET(sockfd, &rset)) {     /* socket is readable */
            if (Readline(sockfd, recvline, MAXLINE) == 0) {
                err_quit("str_cli: server terminated prematurely");
            }
            Fputs(recvline, stdout);
        }

        if (FD_ISSET(fileno(fp), &rset)) {   /* input is readable */
            if (Fgets(sendline, MAXLINE, fp) == NULL) {
                return;
            }
            Writen(sockfd, sendline, MAXLINE);
        }
    }
}

int main(int argc, char **argv)
{
    int    sockfd;
    struct sockaddr_in serveraddr;

    if (argc != 2) {
        err_quit("usage：./tcpcli01 <IPaddress>");
    }

    /* 创建一个 IPv4的字节流类型的TCP套接字 */
    sockfd = Socket(AF_INET, SOCK_STREAM, 0);

    /* 清空结构体 */
    memset(&serveraddr, 0, sizeof(serveraddr));

    /* 初始化结构体 */
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(SERV_PORT);
    Inet_pton(AF_INET, argv[1], &serveraddr.sin_addr);

    /* 连接到 TCP 服务器 */
    Connect(sockfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr));

    str_cli_select(stdin, sockfd);

    exit(0);
}