/**
 * 该程序打印默认的 TCP 和 UDP 的
 * 1、发送缓冲区大小，接收缓冲区大小
 * 2、发送缓冲区低水位标记大小，接收缓冲区低水平标记大小
 */

#include "unp.h"
#include <sys/types.h>
#include <sys/socket.h>

static void doit(int, const char *);

int main(int argc, char **argv)
{
    int tcpsock, udpsock;

    if ((tcpsock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        err_sys("TCP socket error");
    }

    doit(tcpsock, "tcp");

    if ((udpsock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        err_sys("UDP socket error");
    }

    doit(udpsock, "udp");

    exit(0);
}

static void doit(int fd, const char *name)
{
    int         val;
    socklen_t   optlen;

    optlen = sizeof(val);
    if (getsockopt(fd, SOL_SOCKET, SO_SNDBUF, &val, &optlen) < 0) {
        err_sys("SO_SNDBUF getsockopt error");
    }
    printf("%s send buffer size = %d\n", name, val);

    optlen = sizeof(val);
    if (getsockopt(fd, SOL_SOCKET, SO_RCVBUF, &val, &optlen) < 0) {
        err_sys("SO_RCVBUF getsockopt error");
    }
    printf("%s recv buffer size = %d\n", name, val);

    optlen = sizeof(val);
    if (getsockopt(fd, SOL_SOCKET, SO_SNDLOWAT, &val, &optlen) < 0) {
        err_sys("SO_SNDLOWAT getsockopt error");
    }
    printf("%s send low-water mark = %d\n", name, val);

    optlen = sizeof(val);
    if (getsockopt(fd, SOL_SOCKET, SO_RCVLOWAT, &val, &optlen) < 0) {
        err_sys("SO_RCVLOWAT getsockopt error");
    }
    printf("%s receive low-water mark = %d\n", name, val);
}

/**
 * 实验：
[dendi875@192 Chapter-7 Socket Options]$ ./predefaults
tcp send buffer size = 16384
tcp recv buffer size = 87380
tcp send low-water mark = 1
tcp receive low-water mark = 1
udp send buffer size = 188416
udp recv buffer size = 188416
udp send low-water mark = 1
udp receive low-water mark = 1
 */