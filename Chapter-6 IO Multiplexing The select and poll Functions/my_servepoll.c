/**
 * epoll 多路I/0复用 用于 socket 例子：服务端
 */

#include "unp.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>

#define  OPEN_MAX  5000
#define  MAX_EVENTS 5     /* 单个 eopll_wait 调用返回的最大事件数 */
#define  BUFLINE    10

int main(int argc, char **argv)
{
    int                     listenfd, connfd;
    struct  sockaddr_in     serveraddr, clientaddr;
    struct  epoll_event     ev;
    struct  epoll_event     evlist[MAX_EVENTS];
    int                     epfd, ready;
    socklen_t               clientlen;
    char                    buf[BUFLINE];
    int                     len;

    /* 创建一个IPv4的套接字类型为字节流类型的TCP套接字 */
    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    /* 把IPv4的套接字地址结构体变量初始化为0 */
    memset(&serveraddr, 0, sizeof(serveraddr));

    /* 填充结构体变量 */
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(SERV_PORT);
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    /* 把地址和端口绑定到套接字上 */
    Bind(listenfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));

    /* 把未连接的套接字转化成监听套接字 */
    Listen(listenfd, SOMAXCONN);

    /* 创建一个epoll实例，epfd 就代表该实例 */
    if ((epfd = epoll_create(OPEN_MAX)) < 0) {
        err_sys("epoll_create error");
    }

    ev.events = EPOLLIN;   /* 水平触发（默认方式）*/
    // ev.events = EPOLLIN | EPOLLET; /* 边沿触发 */

    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);

    printf("connection from：%s\n", Sock_ntop((struct sockaddr *)&clientaddr, clientlen));

    /* 将 connfd 加入到 epoll 实例感兴趣的文件描述符列表中 */
    ev.data.fd = connfd;
    if ((epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev)) < 0) {
        err_sys("epoll_ctl error");
    }

    /* 执行一个循环，在循环中调用 epoll_wait 来检查 epoll 实例中的就绪的文件描述符 */
    for (;;) {
        ready = epoll_wait(epfd, evlist, MAX_EVENTS, -1);
        if (ready == -1) {
            err_sys("epoll_wait error");
        }

        printf("ready：%d\n", ready);

        if (evlist[0].data.fd == connfd) {
            len = Read(connfd, buf, BUFLINE / 2);
            Write(STDOUT_FILENO, buf, len);
        }
    }

    exit(0);
}

/*
实验：
水平触发 不用等待 epoll_wait 就绪就能把缓冲区中未读完的数据读取了
[dendi875@localhost Chapter-6 IO Multiplexing The select and poll Functions]$ ./my_servepoll
connection from：192.168.100.142:58074
ready：1
aaaa
ready：1
bbbb
ready：1
cccc
ready：1
dddd
ready：1
eeee
ready：1
ffff
^C

边沿触发 必须阻塞等待 socket 中有数据到达触发 epoll_wait 就绪才能读取缓冲区中剩余的数据
[dendi875@localhost Chapter-6 IO Multiplexing The select and poll Functions]$ ./my_servepoll
connection from：192.168.100.142:58073
ready：1
aaaa
ready：1
bbbb
ready：1
cccc
ready：1
dddd
^C
*/