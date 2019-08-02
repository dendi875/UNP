/**
 * 使用 pipe （管道来实现 epoll 的边沿触发和水平触发）
 */

#include "unp.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/epoll.h>

#define BUFLINE  10
#define OPEN_MAX  5000
#define MAX_EVENTS 5    /* 单个 epoll_wait 调用返回的最大事件数 */

int main(int argc, char **argv)
{
    int     pipefd[2], i;
    pid_t   pid;
    char    buf[BUFLINE], c = 'a';
    int     epfd;

    if (pipe(pipefd) < 0) {
        err_sys("pipe error");
    }

    if ((pid = fork()) < 0) {
        err_sys("fork error");
    } else if (pid > 0) {  /* 父进程向管道中写数据 */
        close(pipefd[0]); /* 父进程关闭读端 */

        while (1) {
            // buf = "aaaa\n"
            for (i = 0; i < BUFLINE / 2; i++) {
                buf[i] = c;
            }
            buf[i - 1] = '\n';
            c++;

            // buf = "bbbb\n"
            for (; i < BUFLINE; i++) {
                buf[i] = c;
            }
            buf[i - 1] = '\n';
            c++;

            // buf = "aaaa\nbbbb\n"
            Write(pipefd[1], buf, sizeof(buf));
            sleep(5);
        }

        close(pipefd[1]);
    } else {    /* 子进程向管道中读数据 */
        close(pipefd[1]);

        int                     ready, len;
        struct  epoll_event     ev;
        struct  epoll_event     evlist[MAX_EVENTS];

        /* 创建一个 epoll 实例，epfd 文件描述符就代表该实例 */
        if ((epfd = epoll_create(OPEN_MAX)) < 0) {
            err_sys("epoll_create error");
        }
        ev.data.fd = pipefd[0];
        // ev.events = EPOLLIN;    /* 水平触发（默认）LT（Level Triggered） */
        ev.events = EPOLLIN | EPOLLET; /* 边沿触发 ET（Edge Triggered）*/
        if (epoll_ctl(epfd, EPOLL_CTL_ADD, pipefd[0], &ev) < 0) {
            err_sys("epoll_ctl error");
        }

        /**
         * 执行一个循环，在循环中调用 epoll_wait 来检查 epoll 实例中感兴趣的文件描述符
         */
        for (;;) {
            ready = epoll_wait(epfd, evlist, MAX_EVENTS, -1);
            if (ready == -1) {
                err_sys("epoll_wait error");
            }

            printf("ready：%d\n", ready);
            if (evlist[0].data.fd == pipefd[0]) {
                len = Read(pipefd[0], buf, MAXLINE / 2);
                Write(STDOUT_FILENO, buf, len);
            }
        }

        close(pipefd[0]);
        close(epfd);
    }

    exit(0);
}

/*
实验：水平触发
[dendi875@localhost Chapter-6 IO Multiplexing The select and poll Functions]$ ./my_pipeepoll
ready：1
aaaa
bbbb
ready：1
cccc
dddd
ready：1
eeee
ffff
^C

实验：边沿触发
*/