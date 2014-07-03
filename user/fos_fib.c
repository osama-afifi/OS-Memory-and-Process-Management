// hello, world
#include <inc/lib.h>

int fib(int n)
{
	if(n<=1)
		return n;
	return fib(n-1)+fib(n-2);
}

void _main(void)
{
	char buf[100];
	cprintf("(Press -1 to Stop)\n");
	readline(NULL,buf);
	while(strcmp(buf,"-1"))
			cprintf("Fib of %d : %d\n  (Press -1 to Stop)\n",strtol(buf,NULL,10),fib(strtol(buf,NULL,10)))
			,readline(NULL,buf);
}
