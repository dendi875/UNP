/**
 * 用于简化访问 getaddrinfo 的UDP接口函数。
 * 本函数的参数与 tcp_listen 一样，有一个可选的 hostname和一个必需的service（从而可捆绑其端口号），以及一个可选的指向
 * 某个变量的指针，用于返回套接字地址结构体的大小。
 */
 
int udp_server(const char *host, const char *serv, socklen_t *addrlenp)
{
	int n, sockfd;
	struct addrinfo hints, *res, *ressave;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC; /* 由操作系统决定使用 IPV4或IPV6 */
	hints.ai_flags = AI_PASSIVE; /* 返回的套接字地址将可以用于 bind(2)或accept(2)，返回的套接字地址将包含 “通配地址” */
	hints.ai_socktype = SOCK_DGRAM; /* UDP 用户数据报 */

	if ((n = getaddrinfo(host, serv, &hints, &res)) != 0) {
		err_quit("udp_server error for %s, %s：%s", host, serv, gai_strerror(n));
	}

	ressave = res;

	do {
		sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);	/* 创建一个 IPV4或IPV6的UDP套接字 */
		if (sockfd < 0) {
			continue;	/* error - try next one */
		}

		if (bind(sockfd, res->ai_addr, res->ai_addrlen) == 0) {	/* 把IP地址和端口号捆绑到该套接字上 */
			break;	/* success */
		}
	
		Close(sockfd);	/* bind error - close and try next one */	
	} while ((res = res->ai_next) != NULL);

	if (res == NULL) { 	/* errno from final socket() or bind() */
		err_sys("udp_server error for %s, %s", host, serv);
	}

	if (addrlenp) {
		*addrlenp = res->ai_addrlen;	/* return size of protocol address */
	}

	freeaddrinfo(ressave);

	return(sockfd);
}


int Udp_server(const char *host, const char *serv, socklen_t *addrlenp)
{
	return(udp_server(host, serv, addrlenp));
}


