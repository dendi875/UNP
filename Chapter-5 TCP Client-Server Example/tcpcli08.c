/**
 * 基于 TCP 的客户端/服务器程序－客户端
 *
 * 对两个数求和
 */

#include "unp.h"
#include <sys/types.h>
#include <sys/socket.h>

void str_cli08(FILE *fp, int sockfd)
{
    char    sendline[MAXLINE], recvline[MAXLINE];

    while (Fgets(sendline, MAXLINE, fp) != NULL) {
        Writen(sockfd, sendline, strlen(sendline));

        if (Readline(sockfd, recvline, MAXLINE) == 0) {
            err_quit("str_cli：server terminated prematurely");  /* 服务器过早地终止 */
        }

        Fputs(recvline, stdout);
    }
}

int main(int argc, char **argv)
{
    int     sockfd;
    struct  sockaddr_in  serveraddr;

    if (argc != 2) {
        err_quit("usage：./tcpcli08 <IPaddress>");
    }

    /* 创建一个 IPv4 的TCP 套接字 */
    sockfd = Socket(AF_INET, SOCK_STREAM, 0);

    /* 把 IPv4套接字地址结构体初始化为0 */
    memset(&serveraddr, 0, sizeof(serveraddr));

    /* 填充结构体 */
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(SERV_PORT);
    Inet_pton(AF_INET, argv[1], &serveraddr.sin_addr.s_addr);

    /* 与TCP服务器建立连接 */
    Connect(sockfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr));

    str_cli08(stdin, sockfd);

    exit(0);
}
