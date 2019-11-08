/**
 * 使用封装好的 tcp_listen 来获取时间的服务器端程序的改进版
 */

#include "unp.h"

int main(int argc, char **argv)
{
	int 		listenfd, connfd;
	socklen_t	len, addrlen;
	time_t		ticks;
	char		buff[MAXLINE];
	struct sockaddr_storage	cliaddr;
	
	if (argc == 2) {
		listenfd = Tcp_listen(NULL, argv[1], &addrlen);
	} else if (argc == 3) {
		listenfd = Tcp_listen(argv[1], argv[2], &addrlen);
	} else {
		err_quit("usage：./daytimetcpsrv2 [ <host> ] <service or port>");
	}

	for (;;) {
		len = sizeof(len);
		connfd = Accept(listenfd, (struct sockaddr *)&cliaddr, &len);
		printf("connection from %s\n", Sock_ntop((struct sockaddr *)&cliaddr,  len));

		ticks = time(NULL);
		snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks));
		Write(connfd, buff, sizeof(buff));

		Close(connfd);
	}
	
	exit(0);
}

/**
 * 编译并运行：
 * [dendi875@localhost Chapter-11 Name and Address Conversions]$ gcc -I../lib daytimetcpsrv2.c -o daytimetcpsrv2
 *
 * [dendi875@localhost Chapter-11 Name and Address Conversions]$ ./daytimetcpsrv2 192.168.100.130 9999
 * 在另一台机器上使用 nc 命令来模拟客户端测试
 * [dendi875@localhost Chapter-11 Name and Address Conversions]$ nc 192.168.100.130 9999
 * Fri Oct 11 09:58:18 2019
 */


