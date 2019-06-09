/**
 * 基于 TCP 的回射程序－服务器端（并发服务器）
 */
#include "unp.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>

int main(int argc, char *argv[])
{
    int                     listenfd, connfd;
    struct  sockaddr_in     serveraddr, clientaddr;
    socklen_t               clilen;
    pid_t                   childpid;

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        err_sys("socket error");
    }

    /* 把 IPv4套接字地址结构体初始化为0 */
    memset(&serveraddr, 0, sizeof(serveraddr));

    /* 填充结构体 */
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(SERV_PORT);
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(listenfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) {
        err_sys("error bind");
    }

    if (listen(listenfd, SOMAXCONN) < 0) {
        err_sys("listen error");
    }

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
        } else if (childpid == 0) {      /* 子进程 */
            Close(listenfd);    /* 关闭监听套接字描述符 */
            str_echo(connfd);   /* 处理请求 */
            Close(connfd);      /* 关闭已连接套拼字描述符 */
            exit(0);
        }
        /* 父进程 */
        Close(connfd);          /* 关闭已连接套拼字描述符 */
    }
}
/*
实验：
[dendi875@192 Chapter-5 TCP Client-Server Example]$ ./tcpserv01

查看端口正常
[root@192 ~]# netstat -tlunp | grep 9877
tcp        0      0 0.0.0.0:9877                0.0.0.0:*                   LISTEN      1686/./tcpserv01

使用 nc 来测试
[dendi875@192 ~]$ nc 192.168.17.128 9877
hello,socket
hello,socket
hello,world
hello,world

重新开一个窗口来测试
[dendi875@192 ~]$ nc 192.168.17.128 9877
hello
hello
socket
socket
*/