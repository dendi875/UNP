/**
 * 使用 gethostbyname 和 getservbyname 的时间获取客户程序
 */

#include "unp.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

int main(int argc, char **argv)
{
    int                 sockfd, n;
    struct  hostent     *hp;
    struct  servent     *sp;
    struct  in_addr     inetaddr;
    struct  in_addr     *inetaddrp[2];
    struct  in_addr     **pptr;
    struct  sockaddr_in serveraddr;
    char                recvline[MAXLINE + 1];

    if (argc != 3) {
        err_quit("usage：./daytimetcpcli1 <hostname> <service>");
    }

    if ((hp = gethostbyname(argv[1])) == NULL) {
        if (inet_aton(argv[1], &inetaddr) == 0) {
            err_quit("hostname error for %s：%s", argv[1], hstrerror(h_errno));
        } else {
            inetaddrp[0] = &inetaddr;
            inetaddrp[1] = NULL;
            pptr = inetaddrp;
        }
    } else {
        pptr = (struct in_addr **) hp->h_addr_list;
    }

    if ((sp = getservbyname(argv[2], "tcp")) == NULL) {
        err_quit("getservbyname error for %s", argv[2]);
    }

    for (; *pptr != NULL; pptr++) {
        /* 创建一个IPv4的套接字类型为字节流类型的TCP套接字 */
        sockfd = Socket(AF_INET, SOCK_STREAM, 0);

        /* 把IPv4的TCP套接字地址结构体变量初始化为0 */
        memset(&serveraddr, 0, sizeof(serveraddr));

        /* 填充结构体变量 */
        serveraddr.sin_family = AF_INET;
        serveraddr.sin_port = sp->s_port;
        memcpy(&serveraddr.sin_addr, *pptr, sizeof(struct in_addr));
        printf("trying %s\n", Sock_ntop((struct sockaddr *)&serveraddr, sizeof(serveraddr)));

        if (connect(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == 0) {
            break;
        }
        err_ret("connect error");
        close(sockfd);
    }

    if (*pptr == NULL) {
        err_quit("unable to connect");
    }

    while ( (n = Read(sockfd, recvline, MAXLINE)) > 0) {
        recvline[n] = 0;     /* null terminate */
        Fputs(recvline, stdout);
    }

    exit(0);
}

/*
实验：
[dendi875@localhost Chapter-11 Name and Address Conversions]$ ./daytimetcpcli1 www.example.test daytime
trying 192.168.100.130:13
02 AUG 2019 09:51:55 CST
*/