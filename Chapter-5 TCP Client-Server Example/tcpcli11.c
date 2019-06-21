/**
 * 基于 TCP 的回射程序－客户端
 */

#include "unp.h"
#include <sys/types.h>
#include <sys/socket.h>

void str_cli11(FILE *fp, int sockfd)
{
    char    sendline[MAXLINE], recvline[MAXLINE];

    while (Fgets(sendline, MAXLINE, fp) != NULL) {

        Writen(sockfd, sendline, 1);
        sleep(1);
        Writen(sockfd, sendline+1, strlen(sendline)-1);

        if (Readline(sockfd, recvline, MAXLINE) == 0)
            err_quit("str_cli11: server terminated prematurely");

        Fputs(recvline, stdout);
    }
}

int main(int argc, char **argv)
{
    int                 sockfd;
    struct sockaddr_in  serveraddr;

    if (argc != 2) {
        err_quit("usage：./tcpcli01 <IPaddress>");
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        err_sys("socket error");
    }

    /* 把 IPv4 套接字地址结构体初始化为0 */
    memset(&serveraddr, 0, sizeof(serveraddr));

    /* 填充结构体 */
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(SERV_PORT);
    if (inet_pton(AF_INET, argv[1], &serveraddr.sin_addr) <= 0) {
        err_quit("inet_pton error for %s", argv[1]);
    }

    if (connect(sockfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) {
        err_sys("connect error");
    }

    str_cli11(stdin, sockfd);

    exit(0);
}
