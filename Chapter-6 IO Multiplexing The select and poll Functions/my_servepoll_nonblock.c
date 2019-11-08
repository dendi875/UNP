/**
 * epoll 多路 I/0 用于 socket 的例子，非阻塞模式。服务端
 */

#include "unp.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

#define     OPEN_MAX       5000
#define     MAX_EVENTS     5    /* 单个 */
#define     BUFLINE        10

int main(int argc, char **argv)
{
    int                     listenfd, connfd;
    int                     epfd, ready;
    int                     flags;
    struct  sockaddr_in     serveraddr, clientaddr;
    struct  epoll_event     ev;
    socklen_t               clientlen;
    char                    buf[BUFLINE];
    int                     len;


    /* 创建一个IPv4的套接字类型为字节流类型的TCP套接字 */
    listenfd = Socket(AF_INET, SOCK_STREAM);

    /* 把IPv4的套接字地址结构体变量初始化为0 */
    memset(&serveraddr, 0, sizeof(serveraddr));

    /* 填充结构体变量 */
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(SERV_PORT);
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    /* 把地址和端口捆绑到套接字上 */
    Bind(listenfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));

    /* 把未连接的套接字转换成监听套接字 */
    Listen(listenfd, SOMAXCONN);

    /* 创建一个 epoll 实例，epfd 就代表该实例 */
    if ((epfd = epoll_create(OPEN_MAX)) < 0) {
        err_sys("epoll_create error");
    }
    ev.events = EPOLLIN | EPOLLET;  /* 边沿触发，默认水平触发 */

    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);

    printf("connection from：%s\n", Sock_ntop((struct sockaddr *)&clientaddr, clientlen));

    /* 设置 connfd 为非阻塞 */
    flags = Fcntl(connfd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    Fcntl(connfd, F_SETFL, flags);

    /* 将 connfd 加入到 epoll 实例感兴趣的文件描述符列表中 */
    ev.data.fd = connfd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev) < 0) {
        err_sys("epoll_ctl error");
    }

    /* 执行一个循环，在循环中调用 epoll_wait 来检查 epoll 实例中的文件描述符 */
    for (;;) {
        ready = epoll_wait(epfd, evlist, MAX_EVENTS, -1);
        if (ready == -1) {
            err_sys("epoll_wait error");
        }

        printf("ready：%d\n", ready);

        if (evlist[0].data.fd == connfd) {
            while ((len = read(connfd, buf, BUFLINE / 2)) > 0) {    /* 轮询，非阻塞读 */
                write(STDOUT_FILENO, buf, len);
            }
        }
    }

    exit(0);
}
