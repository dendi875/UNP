/**
 * 使用 select 函数的tcp和udp回射服务器程序
 */

#include "unp.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

void sig_chld(int signo)
{
    pid_t       pid;
    int         status;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        printf("child %d terminated\n", pid);
    }
    return;
}

int main(int argc, char **argv)
{
    int                     listenfd, udpfd, maxfdp1, nready, connfd;
    struct  sockaddr_in     serveraddr, clientaddr;
    const int               on = 1;
    struct  sigaction       act;
    fd_set                  rset;
    pid_t                   childpid;
    socklen_t               clientlen;
    char                    mesg[MAXLINE];
    ssize_t                 n;

    /**
     * 创建一个监听的TCP套接字并捆绑到指定的端口上，设置套接字选项为端口复用
     */

    /* 创建一个IPv4的套接字类型是字节流类型的TCP套接字 */
    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    /* 把IPv4套接字地址结构体变量初始化为0 */
    memset(&serveraddr, 0, sizeof(serveraddr));

    /* 填充结构体 */
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(SERV_PORT);
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    /* 设置套接字选项为重用本地端口 */
    Setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    /* 把地址和端口捆绑到套接字上 */
    Bind(listenfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));

    /* 把未连接的套接字转化成监听套接字 */
    Listen(listenfd, SOMAXCONN);

    /**
     * 创建一个UDP套接字，并捆绑到与TCP套接字相同的端口。
     */
    /* 创建一个IPv4的套接字类型为数据报类型的UDP套接字 */
    udpfd = Socket(AF_INET, SOCK_DGRAM, 0);

    /* 把IPv4的套接字地址结构体变量初始化0 */
    memset(&serveraddr, 0, sizeof(serveraddr));

    /* 填充结构体 */
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(SERV_PORT);
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    /* 把地址和端口捆绑到套接字上 */
    Bind(udpfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));

    /* 捕捉 SIGCHLD 信号 */
    act.sa_handler = sig_chld;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
#ifdef SA_INTERRUPT
    act.sa_flags |= SA_INTERRUPT;
#endif
    if (sigaction(SIGCHLD, &act, NULL) < 0) {
        err_sys("sigaction(SIGCHLD) error");
    }

    /* 初始化一个套接字文件描述符集合，并计算出等待的两个描述符中最大的一个 */
    FD_ZERO(&rset);
    maxfdp1 = max(listenfd, udpfd) + 1;

    /**
     * 1、调用 select 等待监听TCP套接字的可读条件或UDP套接字的可读条件，信号处理函数 sig_chld
     * 有可能会中断慢速系统调用 select，所以需要处理 EINTR 进行手动重试。
     * 2、处理新的客户端的连接，当监听TCP套接字可读时，accept 返回一个新的客户端连接，fork
     * 一个子进程，并在子进程中调用 str_echo函数。
     * 3、处理数据报的到达，如果UDP套接字可读那么表示有一个数据报到达。使用 recfrom 读取并用
     * sendto 把它发回给客户端。
     */
    for (;;) {
        FD_SET(listenfd, &rset);
        FD_SET(udpfd, &rset);
        if ((nready = select(maxfdp1, &rset, NULL, NULL, NULL)) < 0) {
            if (errno == EINTR) {
                continue;
            } else {
                err_sys("select error");
            }
        }

        if (FD_ISSET(listenfd, &rset)) {
            clientlen = sizeof(clientaddr);
            connfd = Accept(listenfd, (struct sockaddr *) &clientaddr, &clientlen);
            if ((childpid = fork()) < 0) {
                err_sys("fork error");
            } else if (childpid == 0) { /* 子进程中 */
                Close(listenfd);    /* 关闭监听套接字 */
                str_echo(connfd);   /* 处理请求 */
                exit(0);
            }
            Close(connfd);      /* 父进程关闭已连接套接子 */
        }

        if (FD_ISSET(udpfd, &rset)) {
            clientlen = sizeof(clientaddr);
            n = Recvfrom(udpfd, mesg, MAXLINE, 0, (struct sockaddr *) &clientaddr, &clientlen);

            Sendto(udpfd, mesg, n, 0, (struct sockaddr *)&clientaddr, clientlen);
        }
    }

    exit(0);
}