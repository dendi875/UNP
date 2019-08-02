/**
 * 基于 TCP的客户端/服务器回射程序－客户端
 *
 * 使用了 select 多路复用
 */

#include "unp.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <netdb.h>

void str_cli_select(FILE *fp, int sockfd)
{
    int     maxfd1;
    fd_set  rset;
    char    sendline[MAXLINE], recvline[MAXLINE];

    FD_ZERO(&rset);
    for (;;) {
        FD_SET(fileno(fp), &rset);
        FD_SET(sockfd, &rset);
        maxfd1 = max(fileno(fp), sockfd) + 1;
        Select(maxfd1, &rset, NULL, NULL, NULL);

        if (FD_ISSET(sockfd, &rset)) {  /* sockfd 是可读的 */
            if (Readline(sockfd, recvline, MAXLINE) == 0) {
                err_quit("str_cli: server terminated prematurely");
            }
            Fputs(recvline, stdout);
        }

        if (FD_ISSET(fileno(fp), &rset)) {  /* 标准输入是可读的 */
            if (Fgets(sendline, MAXLINE, fp) == NULL) {
                return;
            }
            Writen(sockfd, sendline, strlen(sendline));
        }
    }
}

int main(int argc, char **argv)
{
    int                     sockfd;
    struct  sockaddr_in     serveraddr;
    struct  hostent         *hptr;
    char                    **pptr;
    char                    str[INET_ADDRSTRLEN];

    if (argc != 2) {
        err_quit("usage：./tcpcliepoll01 <hostname>");
    }

    /**
     * 调用 gethostbyname 把域名转为ip
     */
    if ((hptr = gethostbyname(argv[1])) == NULL) {
        err_quit("gethostbyname error for host：%s：%s", argv[1], hstrerror(h_errno));
    }
    pptr = hptr->h_addr_list;
    Inet_ntop(hptr->h_addrtype, *pptr, str, sizeof(str));

    /* 创建一个IPv4的字节流类型的TCP套接字 */
    sockfd = Socket(AF_INET, SOCK_STREAM, 0);

    /* 把IPv4套接字地址结构体初始化为0 */
    memset(&serveraddr, 0, sizeof(serveraddr));

    /* 填充结构体 */
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(SERV_PORT);
    Inet_pton(AF_INET, str, &serveraddr.sin_addr);

    /* 连接到 TCP 服务器 */
    Connect(sockfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr));

    str_cli_select(stdin, sockfd);

    exit(0);
}


