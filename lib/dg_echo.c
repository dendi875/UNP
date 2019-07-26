/**
 * dg_echo函数：在数据报套接字上回射文本行
 */

#include "unp.h"

void dg_echo(int sockfd, struct sockaddr *pclientaddr, socklen_t clientlen)
{
    int         n;
    char        mesg[MAXLINE];
    socklen_t   len;

    for (;;) {
        len = clientlen;
        n = Recvfrom(sockfd, mesg, MAXLINE, 0, pclientaddr, &len);

        Sendto(sockfd, mesg, n, 0, pclientaddr, len);
    }
}