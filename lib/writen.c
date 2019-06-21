/* include writen */
#include	"unp.h"

ssize_t writen(int fd, const void *vptr, size_t n) /* Write "n" bytes to a descriptor. */
{
	size_t		nleft;		/* 剩余未写的字节数 */
	ssize_t		nwritten;	/* 本次已写的字节数 */
	const char	*ptr;

	ptr = vptr;
	nleft = n;
	while (nleft > 0) {
		if ( (nwritten = write(fd, ptr, nleft)) <= 0) {
			if (nwritten < 0 && errno == EINTR)
				nwritten = 0;		/* and call write() again */
			else
				return(-1);			/* error */
		}

		nleft -= nwritten;
		ptr   += nwritten;
	}
	return(n);
}
/* end writen */

void Writen(int fd, void *ptr, size_t nbytes)
{
	if (writen(fd, ptr, nbytes) != nbytes)
		err_sys("writen error");
}
