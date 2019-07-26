/**
 * 调用 gethostbyname 并显示返回的信息
 */

#include "unp.h"
#include <unistd.h>
#include <sys/types.h>
#include <netdb.h>

int main(int argc, char **argv)
{
    char            *ptr, **pptr;
    char            str[INET_ADDRSTRLEN];
    struct hostent *hptr;

    while (--argc > 0) {
        ptr = *++argv;
        if ((hptr = gethostbyname(ptr)) == NULL) {
            err_msg("gethostbyname error for host：%s：%s", ptr, hstrerror(h_errno));
            continue;
        }

        printf("official name of host = %s\n", hptr->h_name);
        for (pptr = hptr->h_aliases; *pptr != NULL; pptr++) {
            printf("\talias：%s\n", *pptr);
        }

        switch (hptr->h_addrtype) {
            case AF_INET:
                for (pptr = hptr->h_addr_list; *pptr != NULL; pptr++) {
                    printf("\taddress：%s\n", Inet_ntop(hptr->h_addrtype, *pptr, str, sizeof(str)));
                }
                break;
            default :
                err_ret("unknow address type");
                break;
        }

    }

    exit(0);
}

/**
 * 实验：
 *[dendi875@localhost Chapter-11 Name and Address Conversions]$ ./hostent www.baidu.com
official name of host = www.a.shifen.com
        alias：www.baidu.com
        address：180.101.49.11
        address：180.101.49.12

[dendi875@localhost Chapter-11 Name and Address Conversions]$ ./hostent www.zhangquan.club
official name of host = zhangquan.club
        alias：www.zhangquan.club
        address：45.76.43.49

指定一个不存在的主机名
[dendi875@localhost Chapter-11 Name and Address Conversions]$ ./hostent nosuchname.invalid
gethostbyname error for host：nosuchname.invalid：Unknown host
 */