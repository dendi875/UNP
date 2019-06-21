/**
 * 基于 TCP 的客户端/服务器程序－服务器
 *
 * 对两个数求和
 */

#include "unp.h"
#include <sys/types.h>
#include <sys/socket.h>

void sig_chld(int signo)
{
    pid_t   pid;
    int     status;

    while ( (pid = waitpid(-1, &status, WNOHANG)) > 0) {
        printf("childpid %d terminated\n", pid);
    }
    return;
}

void str_echo08(int sockfd)
{
    long        arg1, arg2;
    ssize_t     n;
    char        line[MAXLINE];

    for ( ; ; ) {
        if ( (n = Readline(sockfd, line, MAXLINE)) == 0)
            return;     /* connection closed by other end */

        if (sscanf(line, "%ld%ld", &arg1, &arg2) == 2)
            snprintf(line, sizeof(line), "%ld\n", arg1 + arg2);
        else
            snprintf(line, sizeof(line), "input error\n");

        n = strlen(line);
        Writen(sockfd, line, n);
    }
}

int main(int argc, char **argv)
{
    int                     listenfd, connfd;
    struct  sockaddr_in     serveraddr, clientaddr;
    socklen_t               clilen;
    pid_t                   childpid;
    struct  sigaction       act;

    /* 创建一个 IPv4的TCP套接字 */
    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    /* 把IPv4套接字地址结构体初始化为0 */
    memset(&serveraddr, 0, sizeof(serveraddr));

    /* 填充结构体 */
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(SERV_PORT);
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    /* 把地址和端口捆绑到套接字上 */
    Bind(listenfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr));

    /* 把未连接的套接字转为已监听的套接字 */
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
            if (errno == EINTR) {
                continue;       /* 被信号中断的低速系统调用，进行重试 */
            } else {
                err_sys("accept error");
            }
        }

        /* 并发服务器 */
        if ( (childpid = fork()) < 0) {
            err_sys("fork error");
        } else if (childpid == 0) {     /* 子进程中 */
            Close(listenfd);        /* 关闭监听套接字 */
            str_echo08(connfd);     /* 处理请求 */
            exit(0);
        }

        /* 父进程中 */
        Close(connfd);
    }

    exit(0);
}
