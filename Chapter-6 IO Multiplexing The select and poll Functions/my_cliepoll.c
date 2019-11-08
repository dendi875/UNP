/**
 * epoll多路I/0复用 用于 socket 例子：客户端
 */

#include "unp.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <netdb.h>

#define  BUFLINE 10

int main(int argc, char **argv)
{
    int                     sockfd;
    int                     i;
    char                    buf[BUFLINE], c = 'a';
    struct  hostent         *hp;
    struct  in_addr         inetaddr;
    struct  in_addr         *inetaddrp[2];
    struct  in_addr         **pptr;
    struct  sockaddr_in     serveraddr;

    if (argc != 2) {
        err_quit("usage：./my_cliepoll <hostname>");
    }

    if ((hp = gethostbyname(argv[1])) == NULL) {
        if (inet_aton(argv[1], &inetaddr) == 0) {
            err_quit("hostname error for %s：%s", argv[1], hstrerror(h_errno));
        } else {
            inetaddrp[0] = &inetaddr;
            inetaddrp[1] = NULL;
        }
    } else {
        pptr = (struct in_addr **) hp->h_addr_list;
    }

    /* 创建一个IPv4的套接字类型是字节流类型的TCP套接字 */
    sockfd = Socket(AF_INET, SOCK_STREAM, 0);

    /* 把 IPv4的套接字地址结构体变量初始化为0 */
    memset(&serveraddr, 0, sizeof(serveraddr));

    /* 填充结构体变量 */
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(SERV_PORT);
    memcpy(&serveraddr.sin_addr, *pptr, sizeof(struct in_addr));

    printf("trying to connect：%s\n", Sock_ntop((struct sockaddr *)&serveraddr, sizeof(serveraddr)));

    /* 连接到 TCP 服务器 */
    Connect(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));

    for (;;) {
        // buf = "aaaa\n"
        for (i = 0; i < BUFLINE / 2; i++) {
            buf[i] = c;
        }
        buf[i - 1] =  '\n';
        c++;

        for (; i < BUFLINE; i++) {
            buf[i] = c;
        }
        buf[i - 1] = '\n';
        c++;

        // buf = "aaaa\nbbbb\n"
        Write(sockfd, buf, sizeof(buf));

        sleep(5);
    }

    close(sockfd);

    exit(0);
}