/* include host_serv */
#include "unp.h"

struct addrinfo *host_serv(const char *node, const char *service, int family, int socktype)
{
	int n;
	struct addrinfo hints, *res;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints->ai_flags = AI_CANONNAME;  /* always return canonical name */
	hints->ai_family = family;		/* AF_UNSPEC, AF_INET, AF_INET6, etc.  */
	hints->ai_socktype = socktype;	/* 0, SOCK_STREAM, SOCK_DGRAM, etc. */

	if ((n = getaddrinfo(node, service, &hints, &res)) != 0) {
		return(NULL);
	}

	return(res);	/* 返回指向链表中第一个节点的指针 */
}

/*
 * There is no easy way to pass back the integer return code from
 * getaddrinfo() in the function above, short of adding another argument
 * that is a pointer, so the easiest way to provide the wrapper function
 * is just to duplicate the simple function as we do here.
 */
 struct addrinfo *Host_serv(const char *node, const char *service, int family, int socktype)
 {
	int n;
	struct addrinfo hints, *res;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints->ai_flags = AI_CANONNAME;
	hints->ai_family = family;
	hints->ai_socktype = socktype;

	if ((n = getaddrinfo(node, service, &hints, &res)) != 0) {
		err_quit("host_serv error for %s, %s, %s",
			(node == NULL) ? "(no node)" : node,
			(service == NULL) ? "(no service name)" : service,
			gai_strerror(n));
	}

	return res; /* 返回指向链表中第一个节点的指针 */
 }

