/**
 * 基于 TCP 的回射程序－客户端，与服务器建立了5个TCP连接
 */

#include "unp.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main(int argc, char **argv)
{
	int 	i, sockfd[5];
	struct  sockaddr_in 	serveraddr;

	if (argc != 2) {
		err_quit("usage：./tcpcli04 <IPaddress>");
	}

	for (i = 0; i < 5; i++) {
		/* 创建一个 IPv4的TCP 套接字 */
		if ((sockfd[i] = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			err_sys("socket error");
		}

		/* 初始化 IPv4套接字地址结构体为0 */
		memset(&serveraddr, 0, sizeof(serveraddr));

		/* 填充结构体 */
		serveraddr.sin_family = AF_INET;
		serveraddr.sin_port = htons(SERV_PORT);
		if (inet_pton(AF_INET, argv[1], &serveraddr.sin_addr) <= 0) {
			err_quit("inet_pton error for %s", argv[1]);
		}

		/* 与TCP服务器建立连接，在成功返回之前会经过三次握手 */
		if (connect(sockfd[i], (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) {
			err_sys("connect error");
		}
	}

	str_cli(stdin, sockfd[0]);

	exit(0);
}

/*
实验：
1、后台启动服务器
[dendi875@192 Chapter-5 TCP Client-Server Example]$ ./tcpserv03 &

2、前台启动客户端
[dendi875@192 Chapter-5 TCP Client-Server Example]$ ./tcpcli04 127.0.0.1

3、netstat 查看套接字状态
[dendi875@192 ~]$ netstat -a | grep 9877
tcp        0      0 *:9877                      *:*                         LISTEN
tcp        0      0 localhost:9877              localhost:44481             ESTABLISHED
tcp        0      0 localhost:44481             localhost:9877              ESTABLISHED
tcp        0      0 localhost:44478             localhost:9877              ESTABLISHED
tcp        0      0 localhost:9877              localhost:44480             ESTABLISHED
tcp        0      0 localhost:44480             localhost:9877              ESTABLISHED
tcp        0      0 localhost:9877              localhost:44478             ESTABLISHED
tcp        0      0 localhost:44482             localhost:9877              ESTABLISHED
tcp        0      0 localhost:9877              localhost:44479             ESTABLISHED
tcp        0      0 localhost:44479             localhost:9877              ESTABLISHED
tcp        0      0 localhost:9877              localhost:44482             ESTABLISHED

4、ps 查看进程状态
[dendi875@192 ~]$ ps -t pts/0 -o pid,ppid,tty,state,command
  PID  PPID TT       S COMMAND
 1691  1690 pts/0    S -bash
 1771  1691 pts/0    S ./tcpserv03
 1923  1691 pts/0    S ./tcpcli04 127.0.0.1
 1924  1771 pts/0    S ./tcpserv03
 1925  1771 pts/0    S ./tcpserv03
 1926  1771 pts/0    S ./tcpserv03
 1927  1771 pts/0    S ./tcpserv03
 1928  1771 pts/0    S ./tcpserv03

5、客户端输入字符
[dendi875@192 Chapter-5 TCP Client-Server Example]$ ./tcpcli04 127.0.0.1
hello
hello
ctrl+d
[dendi875@192 Chapter-5 TCP Client-Server Example]$ child 1924 terminated

6、再查看进程状态
[dendi875@192 ~]$ ps -t pts/0 -o pid,ppid,tty,state,command
  PID  PPID TT       S COMMAND
 1691  1690 pts/0    S -bash
 1771  1691 pts/0    S ./tcpserv03
 1925  1771 pts/0    Z [tcpserv03] <defunct>
 1926  1771 pts/0    Z [tcpserv03] <defunct>
 1927  1771 pts/0    Z [tcpserv03] <defunct>
 1928  1771 pts/0    Z [tcpserv03] <defunct>

可以看到只有一个子进程被回收了，还有4个僵尸进程
*/