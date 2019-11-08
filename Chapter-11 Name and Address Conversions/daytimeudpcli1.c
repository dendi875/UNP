
/**
 * 使用 udp_client 的 UDP 时间获取客户端程序
 *
 * 我们调用 udp_client 函数，然后显示将向其发送UDP数据报的服务器的IP地址和端口号。发送一个1字节
 * 的数据报后读取并显示应答数据报。
 */

 #include "unp.h"

 int main(int argc, char **argv)
 {
	int 				n, sockfd;
	char 				recvline[MAXLINE + 1];
	socklen_t			salen;
	struct sockaddr 	*sa;

	if (argc != 3) {
		err_quit("usage：./daytimeudpcli1 <hostname/IPaddress> <service/port#>");
	}

	sockfd = Udp_client(argv[1], argv[2], &sa, &salen);

	printf("sending to %s\n", Sock_ntop_host(sa, salen));

	Sendto(sockfd, "", 1, 0, sa, salen); /* 发送一个字节的UDP数据报 */
	
	n = Recvfrom(sockfd, recvline, MAXLINE, 0, NULL, NULL);
	recvline[n] = '\0'; /* null terminate */
	Fputs(recvline, stdout);
	
	exit(0);
 }

 /**
  * 编译并运行：
  * [dendi875@localhost Chapter-11 Name and Address Conversions]$ ./daytimeudpcli1 www.example.test daytime
  * sending to 192.168.100.145
  * 28 OCT 2019 17:39:22 CST
  */

