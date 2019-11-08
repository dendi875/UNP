#include "unp.h"

/**
* 1、创建一个 TCP 套接字
* 2、把地址和端口绑定到该套接字上
* 3、把未连接的套接字转成已监听的套接字上
*/

int tcp_listen(const char *host, const char *serv, socklen_t *addrlenp)
{
	int 			n, listenfd;
	const int 		on = 1;
	struct addrinfo hints, *res, *ressave;

	memset(&hints, 0, sizeof(hints)); /* 把结构体清空0 */
	hints.ai_flags = AI_PASSIVE; /* 由操作系统选择服务端的ip，类似 INADDR_ANY 作用 */
	hints.ai_family = AF_UNSPEC; /* 由操作系统来选择是 IPv4还是IPv6 */
	hints.ai_socktype = SOCK_STREAM; /* TCP 套接字 */

	/* 调用 getaddrinfo 把域名和服务名转换成网络地址，网络地址里包含了IP和端口 */
	if ((n = getaddrinfo(host, serv, &hints, &res)) != 0) {
		err_quit("tcp_listen error for %s, %s：%s", host, serv, gai_strerror(n));
	}

	ressave = res;

	do {
		/* 创建一个套接字类型是字节流类型的 TCP 套接字 */
		listenfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (listenfd < 0) {
			continue; /* error, try next one */
		}
		
		/* 设置套接字选项为端口复用 */
		Setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

		/* 把通用套接字地址结构体中的 IP和端口 绑定到套接字上 */
		if (bind(listenfd, res->ai_addr, res->ai_addrlen) == 0) {
			break; /* success */
		}

		Close(listenfd);	/* bind error, close and try next one */
	} while ((res = res->ai_next )!= NULL);

	if (res == NULL) {	/* errno from final socket() or bind() */
		err_sys("tcp_listen error for %s, %s", host, serv);
	}

	/* 把未连接的套接字转换成监听套接字 */
	Listen(listenfd, SOMAXCONN);

	/* 返回相应协议地址的大小 */
	if (addrlenp) {
		*addrlenp = res->ai_addrlen;
	}

	freeaddrinfo(ressave);

	return listenfd;
}

/*
 * We place the wrapper function here, not in wraplib.c, because some
 * XTI programs need to include wraplib.c, and it also defines
 * a Tcp_listen() function.
 */
int Tcp_listen(const char *host, const char *serv, socklen_t *addrlenp)
{
	return(tcp_listen(host, serv, addrlenp));	
}

