/**
 * TCP 回射客户端程序：str_cli 函数
 *
 * 1、fgets 读入一个文本，writen 把该文本行发送给服务器
 * 2、readline 从服务器读入回射行，fputs 把它写到标准输出
 */

#include "unp.h"

void str_cli(FILE *fp, int sockfd)
{
	char 	sendline[MAXLINE], recvline[MAXLINE];

	while (Fgets(sendline, MAXLINE, fp) != NULL) {
		Writen(sockfd, sendline, strlen(sendline));

		if (Readline(sockfd, recvline, MAXLINE) == 0) {
			err_quit("str_cli：server terminated prematurely");	/* 服务器过早地终止 */
		}

		Fputs(recvline, stdout);
	}
}