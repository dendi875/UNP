/**
 * 基于 TCP 的客户端/服务器程序－服务器端
 */
#include "unp.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>

void sig_chld(int signo)
{
    pid_t   pid;
    int     status

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        printf("childpid %d terminated\n", pid);
    }
    return;
}

int main(int argc, char **argv)
{
    int                     listenfd, connfd;
    struct sockaddr_in      serveraddr, clientaddr;
    socklen_t               clientlen;
    pid_t                   pid;
    struct sigaction        act;

    /* 创建一个 IPv4的字节流的TCP套接字文件描述符 */
    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    /* 把IPv4 套接字地址结构初始化为0 */
    memset(&serveraddr, 0, sizeof(serveraddr));

    /* 填充结构体 */
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(SERV_PORT);
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    /* 把地址和端口捆绑到套接字上 */
    Bind(listenfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));

    /* 把未连接的套接字转成监听套接字 */
    Listen(listenfd, SOMAXCONN);

    /* 注册 SIGCHLD 信号捕捉函数，中断的慢速系统调用不会重试　*/
    act.sa_handler = sig_chld;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
#ifdef SA_INTERRUPT
    act.sa_flags |= SA_INTERRUPT;
#endif
    if (sigaction(SIGCHLD, &act, NULL) < 0) {
        err_sys("sigaction(SIGCHLD) error");
    }

    for (;;) {
        clientlen = sizeof(clientaddr);
        if ((connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen)) < 0) {
            if (errno == EINTR) {
                continue;    /* 被信号中断的系统调用进行重试 */
            } else {
                err_sys("accept error");
            }
        }

        if ((pid = fork()) < 0) {
            err_sys("fork error");
        } else if (pid == 0) {  /* 子进程中 */
            Close(listenfd);
            str_echo(connfd);
            exit(0);
        }
        /* 父进程中 */
        Close(connfd);
    }

    exit(0);
}