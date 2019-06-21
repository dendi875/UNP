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

/*
实验：
1、后台启动服务器
[dendi875@192 Chapter-5 TCP Client-Server Example]$ ./tcpserv04 &

2、前台启动客户端
[dendi875@192 Chapter-5 TCP Client-Server Example]$ ./tcpcli04 127.0.0.1

3、netstat 查看套接字状态
[dendi875@192 ~]$ netstat -a | grep 9877
tcp        0      0 *:9877                      *:*                         LISTEN
tcp        0      0 localhost:44486             localhost:9877              ESTABLISHED
tcp        0      0 localhost:44487             localhost:9877              ESTABLISHED
tcp        0      0 localhost:44485             localhost:9877              ESTABLISHED
tcp        0      0 localhost:44483             localhost:9877              ESTABLISHED
tcp        0      0 localhost:9877              localhost:44487             ESTABLISHED
tcp        0      0 localhost:9877              localhost:44484             ESTABLISHED
tcp        0      0 localhost:44484             localhost:9877              ESTABLISHED
tcp        0      0 localhost:9877              localhost:44483             ESTABLISHED
tcp        0      0 localhost:9877              localhost:44485             ESTABLISHED
tcp        0      0 localhost:9877              localhost:44486             ESTABLISHED

4、ps 查看进程状态
 PID  PPID TT       S COMMAND
 1691  1690 pts/0    S -bash
 2142  1691 pts/0    S ./tcpserv04
 2146  1691 pts/0    S ./tcpcli04 127.0.0.1
 2147  2142 pts/0    S ./tcpserv04
 2148  2142 pts/0    S ./tcpserv04
 2149  2142 pts/0    S ./tcpserv04
 2150  2142 pts/0    S ./tcpserv04
 2151  2142 pts/0    S ./tcpserv04

5、客户端输入字符
[dendi875@192 Chapter-5 TCP Client-Server Example]$ ./tcpcli04 127.0.0.1
hello,world
hello,world
ctrl+d
[dendi875@192 Chapter-5 TCP Client-Server Example]$ child 2147 terminated
child 2148 terminated
child 2149 terminated
child 2150 terminated
child 2151 terminated

6、再查看进程状态
[dendi875@192 ~]$ ps -t pts/0 -o pid,ppid,tty,state,command
  PID  PPID TT       S COMMAND
 1691  1690 pts/0    S -bash
 2142  1691 pts/0    S ./tcpserv04

7、再查看套接字状态
[dendi875@192 ~]$ netstat -a | grep 9877
tcp        0      0 *:9877                      *:*                         LISTEN
tcp        0      0 localhost:44486             localhost:9877              TIME_WAIT
tcp        0      0 localhost:44487             localhost:9877              TIME_WAIT
tcp        0      0 localhost:44485             localhost:9877              TIME_WAIT
tcp        0      0 localhost:44483             localhost:9877              TIME_WAIT
tcp        0      0 localhost:44484             localhost:9877              TIME_WAIT

8、过一会再次查看套接字状态
[dendi875@192 ~]$ netstat -a | grep 9877
tcp        0      0 *:9877                      *:*                         LISTEN
*/
