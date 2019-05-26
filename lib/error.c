#include "unp.h"

#include <stdarg.h>
#include <syslog.h>  /* for syslog() */

int daemon_proc;	/*set nonzero by daemon_init() */

static void	err_doit(int, int, const char *, va_list);

/**
* 系统调用相关的非致命错误.
* 打印信息并返回.
*/
void err_ret(const char *fmt, ...)
{
    /**
     * 为了使函数体可以访问到fmt后面的实参，必须声明一个这种类型的变量
     */
    va_list     ap;

    /**
     * 指明了参数列表中可变参数开始的位置（这里从fmt后边开始）
     * 带有可变数量参数的函数必须至少有一个“正常的”形式参数；
     * 省略号总是出现在形式参数列表的最后
     */
    va_start(ap, fmt);
    err_doit(1, LOG_INFO, fmt, ap);
    va_end(ap);
    return;
}

/**
 * 系统调用相关的致命错误.
 * 打印信息并终止
 */
void err_sys(const char *fmt, ...)
{
    va_list     ap;

    va_start(ap, fmt);
    err_doit(1, LOG_ERR, fmt, ap);
    va_end(ap);
    exit(1);
}

/**
 * 系统调用相关的致命错误.
 * 打印信息，产生core dupm，然后终止
 */
void err_dump(const char *fmt, ...)
{
    va_list     ap;

    va_start(ap, fmt);
    err_doit(1, LOG_ERR, fmt, ap);
    va_end(ap);
    abort();    /* dump core and terminate */
    exit(1);    /* shouldn't get here */
}

/**
 * 系统调用无关的非致命错误.
 * 打印信息并返回.
 */
void err_msg(const char *fmt, ...)
{
    va_list    ap;

    va_start(ap, fmt);
    err_doit(0, LOG_INFO, fmt, ap);
    va_end(ap);
    return;
}

/**
 * 系统调用无关的致命错误.
 * 打印信息并终止.
 */
void err_quit(const char *fmt, ...)
{
    va_list     ap;

    va_start(ap, fmt);
    err_doit(0, LOG_ERR, fmt, ap);
    va_end(ap);
    exit(1);
}

/**
 * 打印信息并返回到调用者处.
 * 调用者指定 "errnoflag" and "level".
 */
static void err_doit(int errnoflag, int level, const char *fmt, va_list ap)
{
    int     errno_save, n;
    char    buf[MAXLINE];

    errno_save = errno;
#ifdef  HAVE_VSNPRINTF
    vsnprintf(buf, sizeof(buf), fmt, ap);
#else
    vsprintf(buf, fmt, ap);
#endif
    n = strlen(buf);
    if (errnoflag) {
        snprintf(buf + n, sizeof(buf)-n, ": %s", strerror(errno_save));
    }
    strcat(buf, "\n");

    if (daemon_proc) {
        syslog(level, buf);
    } else {
        fflush(stdout);
        fputs(buf, stderr);
        fflush(stderr);
    }
    return;
}