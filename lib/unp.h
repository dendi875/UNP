/* include unph */

#ifndef _unp_h
#define _unp_h

#include "../config.h"

#include <sys/types.h>	/* 基本系统数据类型 */
#include <sys/socket.h>	/* 基本 socket 定义*/

#if TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#else
#if HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <time.h>
#endif /* HAVE_SYS_TIME_H */
#endif /* TIME_WITH_SYS_TIME */

#include	<netinet/in.h>	/* sockaddr_in{} and other Internet defns */
#include	<arpa/inet.h>	/* inet(3) functions */
#include	<errno.h>
#include	<fcntl.h>		/* for nonblocking */
#include	<netdb.h>
#include	<signal.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<sys/stat.h>	/* for S_xxx file mode constants */
#include	<sys/uio.h>		/* for iovec{} and readv/writev */
#include	<unistd.h>
#include	<sys/wait.h>
#include	<sys/un.h>		/* for Unix domain sockets */

#ifdef	HAVE_SYS_SELECT_H
# include	<sys/select.h>	/* for convenience */
#endif

/* 常值 */
#define MAXLINE		4096	/* 文件行的最大长度 */
#define BUFFSIZE    8192	/* 读写缓冲区的字节数 */


#include "error.c"
#include "wrapunix.c"

#endif /* _unp_h */

