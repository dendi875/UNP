/**
 * 使用 epoll 的TCP回射程序－服务端
 */

#include "unp.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

#define  OPEN_MAX  5000
#define  MAX_EVENTS  5  /* 单个 epoll_wait 调用返回的最大事件数 */

int main(int argc, char **argv)
{
    int                     listenfd, connfd, sockfd;
    int                     epfd, ready, i;
    struct  sockaddr_in     serveraddr, clientaddr;
    const   int             on = 1;
    socklen_t               clientlen;
    struct  epoll_event     ev;
    struct  epoll_event     evlist[MAX_EVENTS];
    ssize_t                 n;
    char                    buf[MAXLINE];

    /* 创建一个IPv4的类型为字节流类型的TCP套接字 */
    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    /* 把IPv4套接字地址结构体变量初始化为0 */
    memset(&serveraddr, 0, sizeof(serveraddr));

    /* 填充结构体变量 */
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(SERV_PORT);
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    /* 设置套接字选项为重用本地端口 */
    Setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    /* 把地址和端口绑定到套接字上 */
    Bind(listenfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr));

    /* 把未连接套接字转化成连接监听套接字 */
    Listen(listenfd, SOMAXCONN);

    /* 创建一个 epoll 实例，epfd 文件描述符就代表改实例 */
    epfd = epoll_create(OPEN_MAX);
    if (epfd < 0) {
        err_sys("epoll_create error");
    }

    /* 将 listenfd 文件描述符添加到 epoll 实例的兴趣列表中，并设置监听的事件为 EPOLLIN */
    ev.data.fd = listenfd;
    ev.events = EPOLLIN;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev) < 0) {
        err_sys("epoll_ctl error");
    }

    /**
     * 执行一个循环，在循环中调用 epoll_wait 来检查 epoll 实例的兴趣列表中的文件描述符，
     * 并处理每个调用返回的事件，如果 epoll_wait 调用成功，程序就再执行一个内层循环检查
     * evlist 中每个已就绪的元素。
     */
    for (;;) {
		ready = epoll_wait(epfd, evlist, MAX_EVENTS, -1);
		if (ready == -1) {
			err_sys("epoll_wait error");
		}

		for (i = 0; i < ready; i++) {
			if (!(evlist[i].events & EPOLLIN)) { 	/* 如果不是读事件，则跳过继续 */
				continue;
			}

            if (evlist[i].data.fd == listenfd) {    /* 如果满足事件的文件描述符是 listenfd */
                clientlen = sizeof(clientaddr);
                connfd = Accept(listenfd, (struct sockaddr *) &clientaddr, &clientlen);
                printf("connection from %s\n", Sock_ntop((struct sockaddr *)&clientaddr, clientlen));

                ev.data.fd = connfd;
                ev.events = EPOLLIN;
                if (epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev) < 0) {
                    err_sys("epoll_ctl error");
                }
            } else {    /* 满足事件的文件描述符是 connfd */
                sockfd = evlist[i].data.fd;
                if ((n = read(sockfd, buf, MAXLINE)) < 0) {
                    if (errno == ECONNRESET) {     /* 连接由客户端发送 RST (reset) 重置 */
                        Close(sockfd);
                        /* 将该文件描述符从感兴趣的列表中移除 */
                        if (epoll_ctl(epfd, EPOLL_CTL_DEL, sockfd, NULL) < 0) {
                            err_sys("epoll_ctl error");
                        }
                    } else {
                        err_sys("read error");
                    }
                } else if (n == 0) {    /* 客户端关闭了连接 */
                    Close(sockfd);
                    if (epoll_ctl(epfd, EPOLL_CTL_DEL, sockfd, NULL) < 0) {
                        err_sys("epoll_ctl error");
                    }
                } else {
                    Writen(sockfd, buf, n);
                }
            }
		}
    }

    Close(listenfd);
    Close(epfd);

    exit(0);
}

