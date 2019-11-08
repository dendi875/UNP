/**
 * epoll 多路 I/0 用于 socket 的例子，非阻塞模式。客户端
 */

#include "unp.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#define  BUFLINE  10

int main(int argc, char **argv)
{
    struct  hostent     *hp;
    struct  in_addr     addr;
    struct  in_addr     *addrp[2];
    struct  in_addr     **pptr;
    int                 sockfd;
    struct  sockaddr_in serveraddr;
    int                 i;
    char                buf[BUFLINE], c = 'a';

    if (argc != 2) {
        err_quit("usage：./my_cliepoll_nonblock <hostname>");
    }

    if ((hp = gethostbyname(argv[1])) == NULL) {
        if (inet_aton(argv[1], &addr) == 0) {
            err_quit("hostname error for %s：%s", argv[1], hstrerror(h_errno));
        } else {
            addrp[0] = &addr;
            addrp[1] = NULL;
        }
    } else {
        pptr = (struct in_addr **) hp->h_addr_list;
    }

    /* 创建一个IPv4的套接字类型为字节流类型的TCP套接字 */
    sockfd = Socket(AF_INET, SOCK_STREAM, 0);

    /* 把IPv4的套接字地址结构体变量初始化为0 */
    memset(&serveraddr, 0, sizeof(serveraddr));

    /* 填充结构体 */
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(SERV_PORT);
    memcpy(&serveraddr.sin_addr, *pptr, sizeof(struct in_addr));

    /* 连接到 TCP 服务器 */
    Connect(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));

    for (;;) {
        // buf = "aaaa\n";
        for (i = 0; i < BUFLINE / 2; i++) {
            buf[i] = c;
        }
        buf[i - 1] = '\n';
        c++;

        for (; i < BUFLINE; i++) {
            buf[i] = c;
        }
        buf[i - 1] = 'n';
        c++;

        // buf = "aaaa\nbbbb\n";
        Write(sockfd, buf, sizeof(buf));

        sleep(5);
    }

    close(sockfd);

    exit(0);
}