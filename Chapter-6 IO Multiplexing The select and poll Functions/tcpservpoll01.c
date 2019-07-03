/**
 * 使用单进程和 poll 的TCP服务器程序
 *
 * nready：就绪的描述符数量
 */

#include "unp.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>
#include <limits.h>

#define  OPEN_MAX  1024

int main(int argc, char **argv)
{
    int                     i, maxi;
    int                     nready;
    ssize_t                 n;
    int                     listenfd, connfd, sockfd;
    struct sockaddr_in      serveraddr, clientaddr;
    struct pollfd           client[OPEN_MAX];
    socklen_t               clientlen;
    char                    buf[MAXLINE];


    /* 创建一个 IPv4字节流类型的TCP套接字 */
    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    /* 把 IPv4套接字地址结构体初始化为0 */
    memset(&serveraddr, 0, sizeof(serveraddr));

    /* 填充结构体 */
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(SERV_PORT);
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    /* 把地址和端口捆绑到套接字上 */
    Bind(listenfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr));

    /* 把未连接的套接字转成监听套接字 */
    Listen(listenfd, SOMAXCONN);

    /**
     * 我们把数组第一个元素的文件描述用于监听套接字，并把其余元素的文件描述符置为-1。
     * 我们还给第一个元素的文件描述符设置 POLLRDNORM 事件，这样当有新的连接准备好被接受
     * 时poll将通知我们。maxi变量含有 client 数组当前正在使用的最大下标值。
     *
     * 如果某个描述符被设置为负值，poll 函数将忽略这样的 pollfd 结构体成员的 events 成员，返回
     * 时将它的 revents 成员的值置为0
     */
    client[0].fd = listenfd;
    client[0].events = POLLRDNORM;
    for (i = 1; i < OPEN_MAX; i++) {
        client[i].fd = -1;
    }
    maxi = 0;


    /**
     * 调用 poll 函数检查新的连接或者现有连接上有数据可读。当一个新的连接被接受后我们
     * 在 client 数组中查找第一个描述符值为负的可用元素。找到后，把新连接的描述符保存
     * 到这个元素中并设置 POLLRDNORM 事件
     */
    for (;;) {
        nready = Poll(client, maxi + 1, -1);

        if (client[0].revents & POLLRDNORM) {   /* 新的连接可以认为是普通数据 */
            clientlen = sizeof(clientaddr);
            connfd = Accept(listenfd, (struct sockaddr *) &clientaddr, &clientlen);

            /* 我们下标从1开始，因为第一个元素固定用于监听套接字 */
            for (i = 1; i < OPEN_MAX; i++) {
                if (client[i].fd < 0) {
                    client[i].fd = connfd;
                    break;
                }
            }

            if (i == OPEN_MAX) {    /* 检查不能超过能打开的文件描述符上限 */
                err_quit("too many clients");
            }

            client[i].events = POLLRDNORM;

            if (i > maxi) {     /* 移动maxi的指向 */
                maxi = i;
            }

            if (--nready <= 0) {    /* 已经没有可读的文件描述符了 */
                continue;
            }
        }

        /**
         * 我们检查两个返回事件 POLLRDNORM 和 POLLERR。其中我们并没有在 events 中设置第二个
         * 事件，因为它在条件成立时总是返回。无论哪种情况，我们都调用 read，当有错误时 read
         * 将返回这个错误。当一个现有连接由它的客户终止时，我们就把它的fd的值置为-1。
         */
        for (i = 1; i <= maxi; i++) {
            if ( (sockfd = client[i].fd) < 0) {
                continue;
            }
            if (client[i].revents & (POLLRDNORM | POLLERR)) {
                if ( (n = read(sockfd, buf, MAXLINE)) < 0) {
                    if (errno == ECONNRESET) { /* 连接由客户端发送 RST (reset) 重置 */
                        Close(sockfd);
                        client[i].fd = -1;
                    } else {
                        err_sys("read error");
                    }
                } else if (n == 0) {    /* 客户端关闭了连接 */
                    Close(sockfd);
                    client[i].fd = -1;
                } else {
                    Writen(sockfd, buf, n);
                }

                if (--nready < 0) {     /* 已经没有可读的文件描述符了 */
                    break;
                }
            }
        }
    }

    exit(0);
}