#include "unp.h"
#include "sum.h"
#include <sys/types.h>
#include <sys/socket.h>

void str_cli09(FILE *fp, int sockfd)
{
    char            sendline[MAXLINE];
    struct  args    args;
    struct  result  result;

    while (Fgets(sendline, MAXLINE, fp) != NULL) {
        if (sscanf(sendline, "%ld%ld", &args.arg1, &args.arg2) != 2) {
            printf("invalid input: %s", sendline);
            continue;
        }
        Writen(sockfd, &args, sizeof(args));

        if (Readn(sockfd, &result, sizeof(result)) == 0) {
            err_quit("str_cli: server terminated prematurely");
        }

        printf("%ld\n", result.sum);
    }
}

int main(int argc, char **argv)
{
    int sockfd;
    struct  sockaddr_in    serveraddr;

    if (argc != 2) {
        err_quit("usage：./tcpcli09 <IPaddress>");
    }

    /* 创建一个 IPv4 字节流类型的套接字 */
    sockfd = Socket(AF_INET, SOCK_STREAM, 0);

    /* 初始化 IPv4套接字地址结构体变量为0 */
    memset(&serveraddr, 0, sizeof(serveraddr));

    /* 填充结构体 */
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(SERV_PORT);
    Inet_pton(AF_INET, argv[1], &serveraddr.sin_addr);

    /* 连接到 TCP 服务器 */
    Connect(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));

    str_cli09(stdin, sockfd);

    exit(0);
}