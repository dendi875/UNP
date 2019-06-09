/**
 * TCP 回射服务器程序：str_echo 函数
 *
 * read　函数从套接字中读入数据，Writen 函数把其中的内容回射给客户端
 */
#include "unp.h"

void str_echo(int sockfd)
{
    ssize_t     n;
    char        buf[MAXLINE];

again:
    while ((n = read(sockfd, buf, sizeof(buf))) > 0) {
        Writen(sockfd, buf, n);
    }

    if (n < 0 && errno == EINTR) {
        goto again;
    } else if (n < 0) {
        err_sys("str_echo：read error");
    }
}