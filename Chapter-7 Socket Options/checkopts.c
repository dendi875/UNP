/**
 * 检查套接字选项是否支持并获取默认值
 */

#include "unp.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>    /* for TCP_xxx defines */

union val {
    int             i_val;
    long            l_val;
    struct  linger  linger_val;
    struct  timeval timeval_val;
} val;

static char *sock_str_flag(union val *, int);
static char *sock_str_int(union val *, int);
static char *sock_str_linger(union val *, int);
static char *sock_str_timeval(union val *, int);

struct sock_opts {
    const    char    *opt_str;
    int              opt_level;
    int              opt_name;
    char             *(*opt_val_str)(union val *, int);
} sock_opts[] = {
#ifdef  SO_RCVBUF
    {"SO_RCVBUF",           SOL_SOCKET,     SO_RCVBUF,          sock_str_int},
#else
    {"SO_RCVBUF",           0,              0,                  NULL},
#endif

#ifdef  SO_SNDBUF
    {"SO_SNDBUF",           SOL_SOCKET,     SO_SNDBUF,          sock_str_int},
#else
    {"SO_SNDBUF",           0,              0,                  NULL},
#endif

#ifdef  SO_REUSEPORT
    {"SO_REUSEPORT",        SOL_SOCKET,     SO_REUSEPORT,       sock_str_flag},
#else
    {"SO_REUSEPORT",        0,              0,                  NULL},
#endif

    {"SO_TYPE",             SOL_SOCKET,      SO_TYPE,           sock_str_int},

#ifdef  SO_USELOOPBACK
    {"SO_USELOOPBACK",      SOL_SOCKET,      SO_USELOOPBACK,    sock_str_flag},
#else
    {"SO_USELOOPBACK",      0,               0,                 NULL},
#endif

    {"IP_TOS",              IPPROTO_IP,      IP_TOS,            sock_str_int},
    {"IP_TTL",              IPPROTO_IP,      IP_TTL,            sock_str_int},

#ifdef IPV6_DONTFRAG
    {"IPV6_DONTFRAG",       IPPROTO_IPV6,    IPV6_DONTFRAG,     sock_str_flag},
#else
    {"IPV6_DONTFRAG",       0,              0,                  NULL},
#endif

#ifdef IPV6_UNICAST_HOPS
    {"IPV6_UNICAST_HOPS",   IPPROTO_IPV6,   IPV6_UNICAST_HOPS,  sock_str_int},
#else
    {"IPV6_UNICAST_HOPS",   0,              0,                  NULL},
#endif

#ifdef IPV6_V6ONLY
    {"IPV6_V6ONLY",         IPPROTO_IPV6,   IPV6_V6ONLY,        sock_str_flag},
#else
    {"IPV6_V6ONLY",         0,              0,                  NULL},
#endif

    {"TCP_MAXSEG",          IPPROTO_TCP,    TCP_MAXSEG,         sock_str_int},
    {"TCP_NODELAY",         IPPROTO_TCP,    TCP_NODELAY,        sock_str_flag},

#ifdef SCTP_AUTOCLOSE
    {"SCTP_AUTOCLOSE",      IPPROTO_SCTP,   SCTP_AUTOCLOSE,     sock_str_int},
#else
    {"SCTP_AUTOCLOSE",      0,              0,                  NULL},
#endif

#ifdef SCTP_MAXBURST
    {"SCTP_MAXBURST",       IPPROTO_SCTP,   SCTP_MAXBURST,      sock_str_int},
#else
    {"SCTP_MAXBURST",       0,              0,                  NULL},
#endif

#ifdef SCTP_MAXSEG
    {"SCTP_MAXSEG",         IPPROTO_SCTP,   SCTP_MAXSEG,        sock_str_int},
#else
    {"SCTP_MAXSEG",         0,              0,                  NULL},
#endif

#ifdef SCTP_NODELAY
    {"SCTP_NODELAY",        IPPROTO_SCTP,   SCTP_NODELAY,       sock_str_flag},
#else
    {"SCTP_NODELAY",        0,              0,                  NULL},
#endif

    {NULL,                  0,              0,                  NULL}
};

int main(int argc, char **argv)
{
    int                 fd;
    struct  sock_opts   *ptr;
    socklen_t           len;

    for (ptr = sock_opts; ptr->opt_str != NULL; ptr++) {
        printf("%s：", ptr->opt_str);
        if (ptr->opt_val_str == NULL) {
            printf("(undefined)\n");
        } else {
            switch (ptr->opt_level) {
                case    SOL_SOCKET:
                case    IPPROTO_IP:
                case    IPPROTO_TCP:
                            fd = Socket(AF_INET, SOCK_STREAM, 0);
                            break;
#ifdef IPV6
                case    IPPROTO_IPV6:
                            fd = Socket(AF_INET6, SOCK_STREAM, 0);
                            break;
#endif

#ifdef IPPROTO_SCTP
                case    IPPROTO_SCTP:
                            fd = Socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);
                            break;
#endif
                default:
                            err_quit("Can't create fd for level %d\n", ptr->opt_level);
            }

            len = sizeof(val);
            if (getsockopt(fd, ptr->opt_level, ptr->opt_name, &val, &len) == -1) {
                err_ret("getsockopt error");
            } else {
                printf("default = %s\n", (*ptr->opt_val_str)(&val, len));
            }

            Close(fd);
        }
    }

    exit(0);
}

static char strres[128];

static char *sock_str_flag(union val *ptr, int len)
{
    if (len != sizeof(int)) {
        snprintf(strres, sizeof(strres), "size (%d) not sizeof(int)", len);
    } else {
        snprintf(strres, sizeof(strres), "%s", (ptr->i_val == 0) ? "off" : "on");
    }
    return(strres);
}

static char *sock_str_int(union val *ptr, int len)
{
    if (len != sizeof(int)) {
        snprintf(strres, sizeof(strres), "size (%d) not sizeof(int)", len);
    } else {
        snprintf(strres, sizeof(strres), "%d", ptr->i_val);
    }
    return(strres);
}

static char *sock_str_linger(union val *ptr, int len)
{
    struct  linger  *lptr = &ptr->linger_val;

    if (len != sizeof(struct linger)) {
        snprintf(strres, sizeof(strres), "size (%d) not sizeof(struct linger)", len);
    } else {
        snprintf(strres, sizeof(strres), "l_onoff = %d, l_linger = %d", lptr->l_onoff, lptr->l_linger);
    }
    return(strres);
}

static char *sock_str_timeval(union val *ptr, int len)
{
    struct timeval  *tvptr = &ptr->timeval_val;

    if (len != sizeof(struct timeval)) {
        snprintf(strres, sizeof(strres), "size (%d) not sizeof(struct timeval)", len);
    } else {
        snprintf(strres, sizeof(strres), "%d sec, %d usec", tvptr->tv_sec, tvptr->tv_usec);
    }
    return(strres);
}

/*
实验：
[dendi875@localhost Chapter-7 Socket Options]$ ./checkopts
SO_RCVBUF：default = 87380
SO_SNDBUF：default = 16384
SO_REUSEPORT：getsockopt error: Protocol not available
SO_TYPE：default = 1
SO_USELOOPBACK：(undefined)
IP_TOS：default = 0
IP_TTL：default = 64
IPV6_DONTFRAG：(undefined)
IPV6_UNICAST_HOPS：default = 64
IPV6_V6ONLY：default = off
TCP_MAXSEG：default = 536
TCP_NODELAY：default = off
SCTP_AUTOCLOSE：(undefined)
SCTP_MAXBURST：(undefined)
SCTP_MAXSEG：(undefined)
SCTP_NODELAY：(undefined)
*/