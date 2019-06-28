/**
 * Socket wrapper functions.
 */

#include "unp.h"

void Close(int fd)
{
	if (close(fd) == -1) {
		err_sys("close error");
	}
}

ssize_t Read(int fd, void *buf, size_t count)
{
    ssize_t    n;

    if ((n = read(fd, buf, count)) == -1) {
        err_sys("read error");
    }
    return(n);
}

ssize_t Write(int fd, const void *buf, size_t count)
{
    ssize_t    n;

    if ((n = write(fd, buf, count)) != count) {
        err_sys("write error");
    }
    return(n);
}