#include "unp.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
	int					listenfd, connfd;
	struct sockaddr_in  serveraddr, clientaddr;
	socklen_t			len;
	char				buff[MAXLINE];
	time_t				ticks;

	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		err_sys("socket error");
	}

	memset(&serveraddr, 0, sizeof(serveraddr));	/* 把 IPv4套接字地址结构体 初始化为0 */

	/**
	 * 填充套接字
	 */
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(13);	/* 本地端口转成网络端口（二进制形式） */
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);


	/* 把ip地址和端口捆绑到套接字上 */
	if (bind(listenfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) {
		err_sys("bind error");
	}

	if (listen(listenfd, SOMAXCONN) < 0) {
		err_sys("error listen");
	}

	for (;;) {
		len = sizeof(clientaddr);
		again:
		if ((connfd = accept(listenfd, (struct sockaddr *) &clientaddr, &len)) < 0) { /* 返回一个已连接的套接字 */
			#ifdef	EPROTO
					if (errno == EPROTO || errno == ECONNABORTED) {
			#else
					if (errno == ECONNABORTED) {
						goto again;
			#endif
					} else {
						err_sys("accept error");
					}
		}

		printf("connection from %s, prot %d\n",
			inet_ntop(AF_INET, &clientaddr.sin_addr, buff, sizeof(buff)), ntohs(clientaddr.sin_port)
			);

		ticks = time(NULL);

		snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks));

		if (write(connfd, buff, strlen(buff)) != strlen(buff)) {
			err_sys("write error");
		}

		if (close(connfd) < 0) {
			err_sys("close error");
		}
	}
}

/*
实验:

客户端：
[dendi875@192 Chapter-1-Introduction]$ ./daytimetcpcli 127.0.0.1
Fri Jun  7 23:46:47 2019

[dendi875@192 Chapter-1-Introduction]$ ./daytimetcpcli 192.168.0.107
Fri Jun  7 23:48:14 2019

服务端：
[root@192 Chapter-1-Introduction]# ./daytimetcpsrv1.1
connection from 127.0.0.1, prot 51238

[root@192 Chapter-1-Introduction]# ./daytimetcpsrv1.1
connection from 127.0.0.1, prot 51238
connection from 192.168.0.107, prot 46321
*/