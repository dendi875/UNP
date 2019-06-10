/**
 * 基于 TCP 的回射程序－服务器端（并发服务器）
 *
 * 带有僵尸进程的处理
 */

#include "unp.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>

/**
 * 打印子进程的退出状态
 */
void pr_exit(int status)
{
    if (WIFEXITED(status)) {
        printf("normal termination, exit status = %d\n", WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
        printf("abnormal termination, signal number = %d%s\n",
            WTERMSIG(status),
#ifdef WCOREDUMP
            WCOREDUMP(status) ? "（core file generated）" : "");
#else
            "");
#endif
    } else if (WIFSTOPPED(status)) {
        printf("child stoped, signal number = %dn", WSTOPSIG(status));
    }
}

void sig_chld(int signo)
{
    pid_t   pid;
    int     status;

    pid = wait(&status);
    pr_exit(status);
    return;
}

int main(int argc, char **argv)
{
    int                 listenfd, connfd;
    struct sockaddr_in  serveraddr, clientaddr;
    socklen_t           clilen;
    pid_t               childpid;
    struct sigaction    act;

    /* 创建一个 IPv4 的 TCP 套接字 */
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        err_sys("socket error");
    }

    /* 把 IPv4 套接字地址结构体初始化为0 */
    memset(&serveraddr, 0, sizeof(serveraddr));

    /* 填充结构体 */
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(SERV_PORT);
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    /* 把地址和端口捆绑到套接字 */
    if (bind(listenfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) {
        err_sys("bind error");
    }

    /* 把未连接的套接字转成已监听的套接字 */
    if (listen(listenfd, SOMAXCONN) < 0) {
        err_sys("listen error");
    }

    /**
     * 安装 SIGCHLD 信号处理程序，处理僵尸进程
     *
     * 这里使用 sigaction 的目的是重现被信号中断的系统调用（accept）不会自动重启
     * signal.h 库中的 signal 函数会自动重启
     *
     * man 7 siganl 有说明
     */
    act.sa_handler = sig_chld;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
#ifdef    SA_INTERRUPT
    act.sa_flags |= SA_INTERRUPT;
#endif

    if (sigaction(SIGCHLD, &act, NULL) < 0) {
        err_sys("sigaction(SIGCHLD) error");
    }

    // if (signal(SIGCHLD, sig_chld) == SIG_ERR) {
    //    err_sys("signal error");
    // }

    for (;;) {
        clilen = sizeof(clientaddr);

        again:
        if ((connfd = accept(listenfd, (struct sockaddr *) &clientaddr, &clilen)) < 0) {
            #ifdef  EPROTO
                    if (errno == EPROTO || errno == ECONNABORTED) {
            #else
                    if (errno == ECONNABORTED) {
                        goto again;
            #endif
                    } else {
                        err_sys("accept error");
                    }
        }

        if ((childpid = fork()) < 0) {
            err_sys("fork error");
        } else if (childpid == 0) {     /* 子进程中 */
            Close(listenfd);
            str_echo(connfd);       /* 处理请求 */
            Close(connfd);
            exit(0);
        }

        /* 父进程 */
        Close(connfd);
    }

    exit(0);
}

/*
实验：
1、后台启动服务器
[dendi875@localhost Chapter-5 TCP Client-Server Example]$ ./tcpserv02 &
[1] 7637

2、同一个主机上前台启动客户端
[dendi875@localhost Chapter-5 TCP Client-Server Example]$ ./tcpcli01 127.0.0.1
hello,socket
hello,socket

按 ctrl+d 后
[dendi875@localhost Chapter-5 TCP Client-Server Example]$ normal termination, exit status = 0
accept error: Interrupted system call

[1]+  Exit 1                  ./tcpserv02
*/