#include "unp.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>

int main(int argc, char *argv[])
{
	int 	listenfd, connfd;
	struct  sockaddr_in  serveraddr;
	time_t	ticks;
	char 	buff[MAXLINE];

	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {	/* 返回的是监听套接字描述述 */
		err_sys("socket error");
	}

	memset(&serveraddr, 0, sizeof(serveraddr));		/* 清空结构体 */

	/**
	 * 填充 IPv4 套接字地址结构体变量
	 *
	 * INADDR_ANY：如果服务器有多块网卡，服务器可以在任意网卡接口上接受客户端连接
	 */
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(13);
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

	/* 把 ip地址和端口 捆绑到套接字serveraddr */
	if (bind(listenfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) {
		err_sys("bind error");
	}

	/**
	 * 把一个未连接的套接字（listendf）转换成一个被动套接字，指示内核应该接受指向该套接字
	 * 的连接请求。
	 */
	if (listen(listenfd, SOMAXCONN) < 0) {
		err_sys("listen error");
	}

	for (;;) {
		again:
		if ((connfd = accept(listenfd, NULL, NULL)) < 0) {
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

		ticks = time(NULL);

		snprintf(buff, sizeof(buff), "tcp daytime：%.24s\r\n", ctime(&ticks));

		if (write(connfd, buff, strlen(buff)) != strlen(buff)) {
			err_sys("write error");
		}

		if (close(connfd) == -1) {
			err_sys("close error");
		}
	}
}