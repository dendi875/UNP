/**
 * epoll 反应堆模式的简单实现。客户端
 */

#include "unp.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <netdb.h>

int main(int argc, char **argv)
{
    int                 sockfd;
    struct  hostent     *hp;
    struct  in_addr     addr;
    struct  in_addr     *addrp[2];
    struct  in_addr     **pptr;
    struct  sockaddr_in serveraddr;

    if (argc != 2) {
        err_quit("usage：./my_clipoll_reactor <hostname>");
    }

    /* 把域名转成 ip 地址 */
    if ((hp = gethostbyname(argv[1])) == NULL) {
        if (inet_aton(argv[1], &addr) == 0) { /* 如果是 IPV4 地址 */
            addrp[0] = &addr;
            addrp[1] = NULL;
			pptr = addrp;
        } else {
            err_quit("hostname error for %s：%s\n", argv[1], hstrerror(h_errno));
        }
    } else {
        pptr = (struct in_addr **) hp->h_addr_list;
    }

    /* 创建一个IPv4的套接字类型为字节流类型的TCP套接字 */
    sockfd = Socket(AF_INET, SOCK_STREAM, 0);

    /* 把IPv4的套接字地址结构体变量初始化为0 */
    memset(&serveraddr, 0, sizeof(serveraddr));

    /* 填充结构体变量 */
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(SERV_PORT);
    memcpy(&serveraddr.sin_addr, *pptr, sizeof(struct sockaddr_in));

    /* 连接到TCP服务端 */
    Connect(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));

	str_cli(stdin, sockfd);
	
    exit(0);
}