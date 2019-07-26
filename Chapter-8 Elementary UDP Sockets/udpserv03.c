/**
 * UDP 回射程序－服务端。
 *
 * 重现 UDP 缺乏流量控制
 */

#include "unp.h"
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

static  int    count;

static  void recvfrom_int(int signo)
{
    printf("\nreceived %d datagrams\n", count);
    exit(0);
}

/**
 * 接收数据报并对其计数，并不把数据报回射给客户端。
 */
void dg_echo_loop1(int sockfd, struct sockaddr *pclientaddr, socklen_t clientlen)
{

    char        mesg[MAXLINE];
    socklen_t   len;
    struct sigaction act;      /* 捕捉 SIGINT 信号 */

    act.sa_handler = recvfrom_int;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
#ifdef SA_INTERRUPT
    act.sa_flags |= SA_INTERRUPT;
#endif
    if (sigaction(SIGINT, &act, NULL) < 0) {
        err_sys("sigaction(SIGINT) error");
    }

    for (;;) {
        len = clientlen;
        Recvfrom(sockfd, mesg, MAXLINE, 0, pclientaddr, &len);

        count++;
    }
}

int main(int argc, char **argv)
{
    int    sockfd;
    struct sockaddr_in  serveraddr, clientaddr;

    /* 创建一个IPv4的数据报的UDP套接字 */
    sockfd = Socket(AF_INET, SOCK_DGRAM, 0);

    /* 把IPv4套接字地址结构体变量初始化为0 */
    memset(&serveraddr, 0, sizeof(serveraddr));

    /* 填充结构体变量 */
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(SERV_PORT);
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    /* 把地址和端口捆绑到套接字上 */
    Bind(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));

    dg_echo_loop1(sockfd, (struct sockaddr *)&clientaddr, sizeof(clientaddr));

    exit(0);
}