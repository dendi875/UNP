/*
 * Wrapper functions for our own library functions.
 * Most are included in the source file for the function itself.
 */

#include "unp.h"

void Inet_pton(int af, const char *src, void *dst)
{
    int    n;

    if ( (n = inet_pton(af, src, dst)) < 0) {
        err_sys("inet_pton error for %s", src); /* errno set */
    } else if (n == 0) {
        err_quit("inet_pton error for %s", src); /* errno not set */
    }
}

const char *Inet_ntop(int af, const void *src, char *dst, socklen_t size)
{
    const char *ptr;

    if ( (ptr = inet_ntop(af, src, dst, size)) == NULL) {
        err_sys("inet_ntop");
    }
    return(ptr);
}