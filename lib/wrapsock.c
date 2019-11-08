/**
 * 套拼字包装函数
 */

#include "unp.h"

int Socket(int domain, int type, int protocol)
{
    int    n;

    if ((n = socket(domain, type, protocol)) < 0) {
        err_sys("socket error");
    }
    return(n);
}

void Connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    if (connect(sockfd, addr, addrlen) < 0) {
        err_sys("connect error");
    }
}

void Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    if (bind(sockfd, addr, addrlen) < 0) {
        err_sys("bind error");
    }
}

void Listen(int sockfd, int backlog)
{
    char    *ptr;

    /*4can override 2nd argument with environment variable */
    if ( (ptr = getenv("LISTENQ")) != NULL)
        backlog = atoi(ptr);

    if (listen(sockfd, backlog) < 0){
        err_sys("listen error");
    }
}

int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    int    n;

again:
    if ( (n = accept(sockfd, addr, addrlen)) < 0) {
#ifdef  EPROTO
        if (errno == EPROTO || errno == ECONNABORTED) {

        }
#else
        if (errno == ECONNABORTED) {
            goto again;
        }
#endif
        else {
            err_sys("accept error");
        }
    }
    return(n);
}

int Select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
{
    int n;

    if ((n = select(nfds, readfds, writefds, exceptfds, timeout)) < 0) {
        err_sys("select error");
    }
    return(n);  /* 超时时可以返回0 */
}

#ifdef HAVE_POLL
int Poll(struct pollfd *fds, nfds_t nfds, int timeout)
{
    int n;

    if ( (n = poll(fds, nfds, timeout)) < 0) {
        err_sys("poll error");
    }
    return(n);  /* 超时时可以返回0 */
}
#endif

int Shutdown(int sockfd, int how)
{
    int     n;

    if ( (n = shutdown(sockfd, how)) < 0) {
        err_sys("shutdown error");
    }
    return(n);
}

int Getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen)
{
    int    n;

    if ( (n = getsockopt(sockfd, level, optname, optval, optlen)) < 0) {
        err_sys("getsockopt error");
    }
    return(n);
}

int Setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen)
{
    int   n;

    if ( (n = setsockopt(sockfd, level, optname, optval, optlen)) < 0) {
        err_sys("setsockopt error");
    }
    return(n);
}

ssize_t Recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen)
{
    ssize_t    n;

    if ( (n = recvfrom(sockfd, buf, len, flags, src_addr, addrlen)) < 0) {
        err_sys("recvfrom error");
    }
    return(n);
}

ssize_t Sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen)
{
    ssize_t    n;

    if ( (n = sendto(sockfd, buf, len, flags, dest_addr, addrlen)) != (ssize_t)len) {
        err_sys("sendto error");
    }
    return(n);
}

/**
 * 如果是客户端调用能够获取服务器端的 IP和端口；
 * 如果是服务器端调用能够获取客户端的 IP和端口
 */
int Getpeername(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
	int n;

	if ((n = getpeername(sockfd, addr, addrlen)) < 0) {
		err_sys("getpeername error");
	}
	return(n);
}