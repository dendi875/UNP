#include "unp.h"

/**
 * 1、创建一个 TCP 套接字（可以是 IPv4或IPv6或两者都有，看主机的支持情况）
 * 2、连接到一个服务器
 * 
 * 返回：已连接的 TCP 套接字文件描述符－－成功，不返回－－出错
 */
int tcp_connect(const char *hostname, const char *service)
{
	int n, sockfd;
	struct addrinfo hints, *res, *ressave;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((n = getaddrinfo(hostname, service, &hints, &res)) != 0) {
		err_quit("tcp_connect error for %s, %s：%s", hostname, service, gai_strerror(n));
	}

	ressave = res;

	do {
		sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (sockfd < 0) {
			continue; 	/* ignore this one */
		}

		/* 连接到 TCP 服务器 */
		if (connect(sockfd, res->ai_addr, res->ai_addrlen) == 0) {
			break;
		}

		Close(sockfd);	/* ignore this one */
	} while ((res = res->ai_next) != NULL);

	if (res == NULL) {
		err_sys("tcp_connect error for %s, %s", hostname, service); /* errno set from final connect() */
	}

	freeaddrinfo(ressave);

	return(sockfd);
}

/*
 * We place the wrapper function here, not in wraplib.c, because some
 * XTI programs need to include wraplib.c, and it also defines
 * a Tcp_connect() function.
 */
int Tcp_connect(const char *hostname, const char *service)
{
	return(tcp_connect(hostname, service));
}