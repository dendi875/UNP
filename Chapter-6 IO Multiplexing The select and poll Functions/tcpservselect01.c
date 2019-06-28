/**
 * 使用单进程和select的TCP服务器程序，分以下两步：
 * 1）初始化
 * 2）循环
 *
 * nready：就绪的描述符数量
 */

#include "unp.h"
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char **argv)
{
    int                 i, maxi, maxfd, listenfd, connfd, sockfd;
    int                 nready, client[FD_SETSIZE];
    struct sockaddr_in  serveraddr, clientaddr;
    fd_set              rset, allset;
    socklen_t           clientlen;
    char                str[INET_ADDRSTRLEN], buf[MAXLINE];
    ssize_t             n;

    /**
     * 创建监听套接字并调用select进行初始化
     */
    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    /* 初始化 IPv4套接字地址结构体变量为0 */
    memset(&serveraddr, 0, sizeof(serveraddr));

    /* 填充结构体变量 */
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(SERV_PORT);
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    /* 把地址和端口绑定到套接字上 */
    Bind(listenfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));

    /* 把未连接的套接字转成监听套接字 */
    Listen(listenfd, SOMAXCONN);

    maxfd = listenfd;
    maxi = -1;
    for (i = 0; i < FD_SETSIZE; i++) {
        client[i] = -1;     /* -1 表示是可用的记录项 */
    }
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);

    /**
     * 循环
     */
    for (;;) {
        /* select 等待某个事件发生：或是新客户端连接的建立，或是数据、FIN或RST的到达 */
        rset = allset;
        nready = Select(maxfd + 1, &rset, NULL, NULL, NULL);

        /**
         * 如果监听套接字变为可读，那么已建立一个新的连接。我们调用 accept 并相应地更新数据结构，
         * 使用client数组中第一个未用项记录这个已连接的描述符。就绪描述符数目减1，若其值变为0，
         * 就可以避免进入下一个for循环。这样做让我们可以使用 select 的返回值来避免检查未就绪
         * 的描述符。
         */
        if (FD_ISSET(listenfd, &rset)) {
            clientlen = sizeof(clientaddr);
            connfd = Accept(listenfd, (struct sockaddr *) &clientaddr, &clientlen);
#ifdef  NOTDEF
            printf("new client：%s, prot %d\n",
                Inet_ntop(AF_INET, &clientaddr.sin_addr, str, sizeof(str)), ntohs(clientaddr.sin_port));
#endif
            for (i = 0; i < FD_SETSIZE; i++) {
                if (client[i] < 0) {
                    client[i] = connfd;
                    break;
                }
            }

            if (i == FD_SETSIZE) {
                err_quit("too many clients");
            }

            FD_SET(connfd, &allset);  /* 添加新描述符到集合中 */

            if (connfd > maxfd) {
                maxfd = connfd;
            }

            if (i > maxi) {
                maxi = i;
            }

            if (--nready <= 0) {
                continue;
            }
        }

        /**
         * 对于每个现有的客户连接，我们要测试其描述符是否在 select 返回的描述符集中。如果是就从该客户读
         * 入一行文本并回射给它。如果该客户关闭了连接，那么read将返回0，我们于是相应地更新数据结构。
         *
         * 我们从不减少 maxi 的值。
         */
        for (i = 0; i <= maxi; i++) {
            if ( (sockfd = client[i]) < 0) {
                continue;
            }
            if (FD_ISSET(sockfd, &rset)) {
                if ( (n = Read(sockfd, buf, MAXLINE)) == 0) {   /* 客户端关闭了连接 */
                    Close(sockfd);
                    client[i] = -1;
                    FD_CLR(sockfd, &allset);
                } else {
                    Writen(sockfd, buf, n);
                }

                if (--nready <= 0) {
                    break;
                }
            }
        }
    }

    exit(0);
}