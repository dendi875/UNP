/**
 * 标准 I/O 包裹函数
 */

#include "unp.h"

char *Fgets(char *ptr, int n, FILE *stream)
{
	char 	*rptr;

	if ((rptr = fgets(ptr, n, stream)) == NULL && ferror(stream)) {
		err_sys("fgets error");
	}

	return(rptr);
}

int Fputs(const char *s, FILE *stream)
{
	if (fputs(s, stream) == EOF) {
		err_sys("fputs error");
	}
}