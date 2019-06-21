#include "unp.h"
#include "sum.h"
#include <sys/types.h>
#include <sys/socket.h>

void str_echo09(int sockfd)
{
    ssize_t             n;
    struct  args        args;
    struct  result      result;

    for (;;) {
        if ((n = Readn(sockfd, &args, sizeof(args))) == 0) {
            return;     /* 连接由另一端关闭 */
        }

        result.sum = args.arg1 + args.arg2;
        Writen(sockfd, &result, sizeof(result));
    }
}

void sig_chld(int signo)
{
    /* 回收僵尸进程 */

    pid_t   pid;
    int     status;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        printf("child %d terminated\n", pid);
    }
    return;
}

int main(int argc, char **argv)
{
    int                     listenfd, connfd;
    struct sockaddr_in      serveraddr, clientaddr;
    socklen_t               clilen;
    pid_t                   pid;
    struct sigaction        act;


    /* 创建一个 IPv4字节流套接字 */
    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    /* 初始化 IPv4 套接字地址结构体变量为0 */
    memset(&serveraddr, 0, sizeof(serveraddr));

    /* 填充结构体 */
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(SERV_PORT);
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    /* 把地址和端口捆绑到套接字上 */
    Bind(listenfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));

    /* 把未连接的套接字转为监听套接字 */
    Listen(listenfd, SOMAXCONN);

    /* 安装 SIGCHLD 信号处理程序 */
    act.sa_handler = sig_chld;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
#ifdef  SA_INTERRUPT
    act.sa_flags |= SA_INTERRUPT;
#endif
    if (sigaction(SIGCHLD, &act, NULL) < 0) {
        err_sys("sigaction(SIGCHLD) error");
    }

    for (;;) {
        clilen = sizeof(clientaddr);
        if ( (connfd = accept(listenfd, (struct sockaddr *) &clientaddr, &clilen)) < 0) {
            if (errno = EINTR) {
                continue;   /* 被信号中断的低速系统调用进行自动重试 */
            } else {
                err_sys("accept error");
            }
        }
        /* 多进程实现并发服务器 */
        if ((pid = fork()) < 0) {
            err_sys("fork error");
        } else if (pid == 0) {  /* 子进程 */
            Close(listenfd);    /* 关闭监听套接字描述符 */
            str_echo09(connfd); /* 处理请求 */
            exit(0);
        }
        /* 父进程 */
        Close(connfd);  /* 关闭已连接的套接字 */
    }

    exit(0);
}