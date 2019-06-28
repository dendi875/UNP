/**
 * 基于 TCP 的客户端/服务器程序－客户端
 *
 * 使用了 select 和 shutdown
 */

#include "unp.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>

void str_cli_select02(FILE *fp, int sockfd)
{
    int     maxfdp1, stdineof;
    fd_set  rset;   /* 可读的描述符集合 */
    int     n;
    char    buf[MAXLINE];

    stdineof = 0;
    FD_ZERO(&rset);
    for (;;) {
        if (stdineof == 0) {
            FD_SET(fileno(fp), &rset);
        }
        FD_SET(sockfd, &rset);
        maxfdp1 = max(fileno(fp), sockfd) + 1;
        Select(maxfdp1, &rset, NULL, NULL, NULL);

        if (FD_ISSET(sockfd, &rset)) {      /* 套接字是可读的 */
            if ((n = Read(sockfd, buf, MAXLINE)) == 0) {
                if (stdineof == 1) {
                    return;     /* 正常终止 */
                } else {
                    err_quit("str_cli_select02：server terminated prematurely");
                }
            }
            Write(fileno(stdout), buf, n);
        }

        if (FD_ISSET(fileno(fp), &rset)) {  /* 标准输入是可读的 */
            if ( (n = Read(fileno(fp), buf, MAXLINE) ) == 0) {
                stdineof = 1;
                Shutdown(sockfd, SHUT_WR);  /* 发送FIN */
                FD_CLR(fileno(fp), &rset);
                continue;
            }
            Write(sockfd, buf, n);
        }
    }
}

int main(int argc, char **argv)
{
    int    sockfd;
    struct sockaddr_in  serveraddr;

    if (argc != 2) {
        err_quit("usage：./tcpcli02 <IPaddress>");
    }

    /* 创建一个IPv4的字节流类型的TCP套接字 */
    sockfd = Socket(AF_INET, SOCK_STREAM, 0);

    /* 把TCP套接字地址结构体变量初始化为0 */
    memset(&serveraddr, 0, sizeof(serveraddr));

    /* 填充结构体 */
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(SERV_PORT);
    Inet_pton(AF_INET, argv[1], &serveraddr.sin_addr);

    /* 连接到TCP服务器 */
    Connect(sockfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr));

    str_cli_select02(stdin, sockfd);

    exit(0);
}