#include "unp.h"

/**
 * 协议无关的UDP时间获取服务器程序。
 * 使用了我们自己封装的 udp_server 函数。
 */
int main(int argc, char **argv)
{	
	int 		n;
	int 		sockfd;
	char 		buff[MAXLINE];
	time_t		ticks;
	socklen_t	len;
	struct  sockaddr_storage cliaddr;

	if (argc == 2) {
		sockfd = Udp_server(NULL, argv[1], NULL);
	} else if (argc == 3) {
		sockfd = Udp_server(argv[1], argv[2], NULL);
	} else {
		err_quit("usage：./daytimeudpsrv2 [ <host> ] <service or port>");
	}

	for (;;) {
		len = sizeof(cliaddr);
		n = Recvfrom(sockfd, buff, MAXLINE, 0, (struct sockaddr *) &cliaddr, &len);
		printf("datagram from %s\n", Sock_ntop((struct sockaddr *) &cliaddr, len));

		ticks = time(NULL);
		snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks));
		Sendto(sockfd, buff, strlen(buff), 0, (struct sockaddr *) &cliaddr, len);
	}	
	
	exit(0);
}

/**
 * 编译并运行：
 * [dendi875@localhost Chapter-11 Name and Address Conversions]$ gcc -I../lib daytimeudpsrv2.c -o daytimeudpsrv2
 * [dendi875@localhost Chapter-11 Name and Address Conversions]$ ./daytimeudpsrv2 9877   
 * 换另一台机器使用 `nc` 命令来模拟客户端发送 UDP 数据包
 * [dendi875@localhost Chapter-11 Name and Address Conversions]$ nc -4u 192.168.100.142 9877
 *
 * Tue Oct 29 11:29:36 2019
 */

