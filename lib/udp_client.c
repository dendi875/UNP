/**
 * 对 getaddrinfo 封装后，用于 UDP 客户端的函数
 *
 * 本函数创建一个未连接的 UDP 套接字，是个值－结果函数，返回三项数据
 * 1、返回值是该套接字的描述符
 * 2、saptr 是指向某个（由 udp_client动态分配的）套接字地址结构的（由调用者自行声明的）一个指针的地址，
 * 本函数把目的IP地址和端口存放在这个结构中，用于稍后调用 sendto
 * 3、这个套接字地址结构的大小在 lenp　指向的变量中返回。
 * lenp 这个参数不能是一个空指针（而 tcp_listen 允许最后一个参数是一个空指针），
 * 因为任何 sendto　和 recvfrom 调用都需要知道套接字结构结构的长度
 */

#include "unp.h"

int udp_client(const char *host, const char *serv, struct sockaddr **saptr, socklen_t *lenp)
{
	int n, sockfd;
	struct addrinfo hints, *res, *ressave;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;	/* 由操作系统决定是 IPV4还是IPV6 */
	hints.ai_socktype = SOCK_DGRAM;	/* 数据报套接字 UDP */

	if ((n = getaddrinfo(host, serv, &hints, &res)) != 0) {
		err_quit("udp_client error for %s，%s：%s", host, serv, gai_strerror(n));
	}

	ressave = res;

	do {
		sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (sockfd >= 0) {
			break; /* success */
		}
	} while ((res = res->ai_next) != NULL);

	if (res == NULL) { /* errno set from final socket() */
		err_sys("udp_client error for %s，%s", host, serv);
	}

	*saptr = malloc(sizeof(res->ai_addrlen));
	if (*saptr == NULL) {
		err_sys("malloc error");
	}
	memcpy(*saptr, res->ai_addr, res->ai_addrlen);
	*lenp = res->ai_addrlen;

	freeaddrinfo(ressave);

	return(sockfd);
}

int Udp_client(const char *host, const char *serv, struct sockaddr **saptr, socklen_t *lenptr)
{
	return(udp_client(host, serv, saptr, lenptr));
}

