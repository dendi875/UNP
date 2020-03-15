/**
 * 基于 TCP 的回射程序－服务端
 *
 * 0、并发服务器
 * 1、带有僵尸进程的处理
 * 2、被信号中断的系统调用 accpet 会进行自动重试
 * 3、多个连接终止时能够正确回收所有的子进程
 *
 * 本程序对应 UNP 书上 112 页的描述
 */

#include "unp.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/wait.h>

void sig_chld(int signo)
{
    pid_t pid;
    int   status;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        printf("child %d terminatedn", pid);
    }

    return;
}

int main(int argc, char **argv)
{
    int                 listenfd, connfd;
    struct sockaddr_in  serveraddr, clientaddr;
    struct sigaction    act;
    socklen_t           clilen;
    pid_t               childpid;

    /* 创建一个 IPv4 TCP 的套接字 */
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        err_sys("socket error");
    }

    /* 把 IPv4 套接字地址结构体初始化为0 */
    memset(&serveraddr, 0, sizeof(serveraddr));

    /* 填充结构体 */
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(SERV_PORT);
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY); /* 通配地址 */

    /* 把地址和端口绑定到套接字上 */
    if (bind(listenfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) {
        err_sys("bind error");
    }

    /* 把未连接的套接字转成监听套接字 */
    if (listen(listenfd, SOMAXCONN) < 0) {
        err_sys("listen error");
    }

    /* 安装 SIGCHLD 信号处理程序 */
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
        clilen = sizeof(clientaddr);
        if ((connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clilen)) < 0) {
            if (errno == EINTR) {   /* 信号中断的调用，进行重试 */
                continue;
            } else {
                err_sys("accept error");
            }
        }

        /* fork 子进程来处理连接请求 */
        if ((childpid = fork()) < 0) {
            err_sys("fork error");
        } else if (childpid == 0) { /* 子进程中 */
            Close(listenfd);
            str_echo(connfd);
            exit(0);
        }

        /* 父进程中 */
        Close(connfd);
    }

    exit(0);
}

/**
 * 实验：
 * 1、启动服务器
 * [dendi875@192 Chapter-5 TCP Client-Server Example]$ ./my_tcp_serv_terminated
 * 2、启动客户端，并验证程序正常
 * [dendi875@192 Chapter-5 TCP Client-Server Example]$ ./my_tcp_cli_terminated 192.168.100.145
 * hello
 * hello
 * 3、杀死服务端的子进程，并观察 Wireshark 包
 * [dendi875@192 ~]$ ps -elf | grep tcp
    4 S root      3273  3270  0  80   0 -  9661 ?      19:20 ?        00:00:09 /usr/bin/Xorg :0 -br -verbose -audit 4 -auth /var/run/gdm/auth-for-gdm-FDiGnv/database -nolisten tcp vt1
    0 S dendi875  6798  3900  0  80   0 -   470 -      20:46 pts/0    00:00:00 ./my_tcp_serv_terminated
    1 S dendi875  6799  6798  0  80   0 -   470 -      20:46 pts/0    00:00:00 ./my_tcp_serv_terminated
    0 S dendi875  6847  6823  0  80   0 -  1495 -      20:51 pts/1    00:00:00 grep --color=auto tcp
4、杀死服务端的子进程
[dendi875@192 ~]$ kill -9 6799

5、观察服务端，可以看到服务端经过了 FIN_WAIT1、FIN_WAIT2 状态，客户端经过 CLOSE_WAIT

服务端：
[dendi875@192 ~]$ netstat -a | grep 9877
tcp        0      0 *:9877                      *:*                         LISTEN
tcp        0      0 192.168.100.145:9877        192.168.100.142:53833       FIN_WAIT2

客户端：
[dendi875@192 ~]$ netstat -a | grep 9877
tcp        1      0 192.168.100.142:53833       www.example.test:9877       CLOSE_WAIT

6、客户端在输入文本
[dendi875@192 Chapter-5 TCP Client-Server Example]$ ./my_tcp_cli_terminated 192.168.100.145
hello
hello
another line
str_cli：server terminated prematurely
 */