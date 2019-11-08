/**
 * 使用封装好的 tcp_connect 来获取时间的客户端程序
 */

#include "unp.h"

int main(int argc, char **argv)
{
	int 					sockfd, n;
	struct sockaddr_storage ss;
	socklen_t				addrlen;
	char 					recvline[MAXLINE + 1];	

	if (argc != 3) {
		err_quit("usage：./daytimetcpcli <hostname/IPaddress> <service/port#>");
	}

	sockfd = Tcp_connect(argv[1], argv[2]);
	
	addrlen = sizeof(ss);
	Getpeername(sockfd, (struct sockaddr *)&ss, &addrlen);
	printf("connected to %s\n", Sock_ntop_host((struct sockaddr *)&ss, addrlen));

	while ((n = Read(sockfd, recvline, MAXLINE)) > 0) {
		recvline[n] = 0;
		Fputs(recvline, stdout);
	}
	
	exit(0);
}

/**
 * 编译并运行
 *
 * [dendi875@localhost Chapter-11 Name and Address Conversions]$ gcc -I../lib daytimetcpcli.c -o daytimetcpcli
 * [dendi875@localhost Chapter-11 Name and Address Conversions]$ ./daytimetcpcli www.example.test daytime
 * connected to 192.168.100.130
 * 09 OCT 2019 11:03:30 CST
 */
