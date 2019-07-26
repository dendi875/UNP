/**
 * UDP 回射程序－服务端
 */

#include "unp.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main(int argc, char **argv)
{
    int     sockfd;
    struct  sockaddr_in     serveraddr, clientaddr;

    /* 创建一个IPv4的数据报类型的UDP套接字 */
    sockfd = Socket(AF_INET, SOCK_DGRAM, 0);

    /* 把IPv4套接字地址结构体初始化为0 */
    memset(&serveraddr, 0, sizeof(serveraddr));

    /* 填充结构体 */
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(SERV_PORT);
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    /* 把地址和端口捆绑到套接字上 */
    Bind(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));

    /* 调用 dg_echo 来执行服务器工作 */
    dg_echo(sockfd, (struct sockaddr *)&clientaddr, sizeof(clientaddr));

    exit(0);
}