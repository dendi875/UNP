/**
 * 使用封装好的 tcp_listen 来获取时间的服务器端程序
 */

 #include "unp.h"

 int main(int argc, char **argv)
 {
	int 		listenfd, connfd;
	socklen_t	len;
	time_t		ticks;
	char 		buff[MAXLINE];
	struct sockaddr_storage cliaddr;
	
	if (argc != 2) {
		err_quit("usage：./daytimetcpsrv1 <service or port#>");
	}

	listenfd = Tcp_listen(NULL, argv[1], NULL);

	for (;;) {
		len = sizeof(cliaddr);
		connfd = Accept(listenfd, (struct sockaddr *)&cliaddr, &len);
		printf("connection from %s\n", Sock_ntop((struct sockaddr *)&cliaddr, len));

		ticks = time(NULL);
		snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks));
		Write(connfd, buff, sizeof(buff));
		
		Close(connfd);
	}
	
	exit(0);
 }

 /**
 * 实验：
 * 服务端
 * [dendi875@localhost Chapter-11 Name and Address Conversions]$ gcc -I../lib daytimetcpsrv1.c -o daytimetcpsrv1
 * [dendi875@localhost Chapter-11 Name and Address Conversions]$ ./daytimetcpsrv1 9999
 *
 * 客户端我们用 nc 来模拟
 * [dendi875@localhost ~]$ nc 192.168.100.130 9999
  * Thu Oct 10 19:01:50 2019
  * 换成域名来试试
  * [dendi875@localhost ~]$ grep www.example.test /etc/hosts
  * 192.168.100.130 www.example.test
  * [dendi875@localhost ~]$ nc www.example.test 9999
  * Thu Oct 10 19:03:37 2019
 */

