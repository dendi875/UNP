/**
 * 基于 TCP 的回射程序－服务器端
 *
 * 0、并发服务器
 * 1、带有僵尸进程的处理
 * 2、被信号中断的系统调用 accept 会进行重试
 * 3、多个连接终止时能够正确回收所有的子进程
 */

#include "unp.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/wait.h>

void sig_chld(int signo)
{
	pid_t	pid;
	int 	status;

	while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
		printf("child %d terminated\n", pid);
	}
	return;
}

int main(int argc, char **argv)
{
	int    					listenfd, connfd;
	struct 	sockaddr_in    	serveraddr, clientaddr;
	socklen_t				clilen;
	pid_t					childpid;
	struct  sigaction		act;

	/* 创建一个 地址族为IPv4的字节流类型的套接字 */
	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		err_sys("socket error");
	}

	/* 把IPv4套接字地址结构体初始化为0 */
	memset(&serveraddr, 0, sizeof(serveraddr));

	/* 填充结构体 */
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(SERV_PORT);
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);	/* 通配地址 */

	/* 把地址和端口捆绑到套接字上 */
	if (bind(listenfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) {
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
#ifdef	SA_INTERRUPT
	act.sa_flags |= SA_INTERRUPT;
#endif

	if (sigaction(SIGCHLD, &act, NULL) < 0) {
		err_sys("sigaction(SIGCHLD) error");
	}

	for (;;) {
		clilen = sizeof(clilen);
		if ((connfd = accept(listenfd, (struct sockaddr *) &clientaddr, &clilen)) < 0) {
			if (errno == EINTR) {	/* 信号中断的调用，进行重试 */
				continue;
			} else {
				err_sys("accept error");
			}
		}

		/* fork 子进程来处理连接请求（并发服务器）*/
		if ((childpid = fork()) < 0) {
			err_sys("fork error");
		} else if (childpid == 0) {		/* 子进程中 */
			Close(listenfd);	/* 关闭监听套接字文件描述符 */
			str_echo(connfd);	/* 处理请求 */
			exit(0);
		}

		/* 父进程中 */
		Close(connfd);
	}

	exit(0);
}