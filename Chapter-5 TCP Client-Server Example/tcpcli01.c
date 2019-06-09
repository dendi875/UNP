/**
 * 基于 TCP 的回射程序－客户端
 */

#include "unp.h"
#include <sys/types.h>
#include <sys/socket.h>

int main(int argc, char **argv)
{
	int 				sockfd;
	struct sockaddr_in	serveraddr;

	if (argc != 2) {
		err_quit("usage：./tcpcli01 <IPaddress>");
	}

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		err_sys("socket error");
	}

	/* 把 IPv4 套接字地址结构体初始化为0 */
	memset(&serveraddr, 0, sizeof(serveraddr));

	/* 填充结构体 */
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(SERV_PORT);
	if (inet_pton(AF_INET, argv[1], &serveraddr.sin_addr) <= 0) {
		err_quit("inet_pton error for %s", argv[1]);
	}

	if (connect(sockfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) {
		err_sys("connect error");
	}

	str_cli(stdin, sockfd);

	exit(0);
}

/*
实验：
1、启动服务器放入后台运行
[dendi875@192 Chapter-5 TCP Client-Server Example]$ ./tcpserv01 &
[1] 2702

2、观察套接字状态
[dendi875@192 Chapter-5 TCP Client-Server Example]$ netstat -a
Active Internet connections (servers and established)
Proto Recv-Q Send-Q Local Address               Foreign Address             State
tcp        0      0 *:9877                      *:*                         LISTEN

3、在同个主机上启动客户端
[dendi875@192 Chapter-5 TCP Client-Server Example]$ ./tcpcli01 127.0.0.1

4、重新开一个窗口使用 netstat 观察套接字状态
[dendi875@192 ~]$ netstat -a
Active Internet connections (servers and established)
Proto Recv-Q Send-Q Local Address               Foreign Address             State
tcp        0      0 *:9877                      *:*                         LISTEN
tcp        0      0 localhost:9877              localhost:36224             ESTABLISHED
tcp        0      0 localhost:36224             localhost:9877              ESTABLISHED

5、使用 ps 观察进程的状态和关系
[dendi875@192 ~]$ ps -t pts/0 -o pid,ppid,tty,stat,args,wchan
  PID  PPID TT       STAT COMMAND                     WCHAN
 1657  1656 pts/0    Ss   -bash                       -
 2702  1657 pts/0    S    ./tcpserv01                 -
 2718  1657 pts/0    S+   ./tcpcli01 127.0.0.1        -
 2719  2702 pts/0    S    ./tcpserv01                 -
 */