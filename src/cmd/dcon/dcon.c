#include <stdio.h>
#include <fcntl.h>
#include "sys/mk68564.h"

#define DEBUG

#ifdef DEBUG
#define DEBUG1(STR,ARG)  printf(STR,ARG);
#else
#define DEBUG1
#endif

char console[] = "/dev/console";
char onstr[] = "on";
char offstr[] = "off";
char debugstr[] = "DebuG";

main(argc, argv)
register int argc;
register char *argv[];
{
	register int fd;
	register int toggle;
	register int arg;

	extern int errno;

	if( setuid(0) < 0)
	{
		printf("You are not ROOT, permission denied!\n");
		exit(0);
	}

	if(argc < 2)
	{
		printf("dcon: not enough arguments\n");
		printf("dcon: syntax: dcon [on][off]\n");
		exit(0);
	}

	arg = 0;

	if(!(strcmp(*++argv, onstr)))
		toggle = CONC_ON;
	else if(!(strcmp(*argv, offstr)))
		toggle = CONC_OFF;
	else if(!(strcmp(*argv, debugstr)))
	{
		toggle = CONC_DEBUG;
		arg = (*++argv)[0];
	}
	else 
	{
		printf("dcon: Invalid argument: %s\n",
			*argv);
		printf("dcon: syntax: dcon [on][off]\n");
		exit(0);
	}
		

	if((fd = open(console, O_RDWR)) < 0)
	{
		printf("dcon: Open error on console, error: %d\n",
			console, errno);
		exit(0);
	}

	if((ioctl(fd, toggle, arg)) < 0)
	{
		close(fd);
		printf("dcon: Ioctl error on console, error: %d\n",
			console, errno);
		printf("dcon: Could not make tty00 and tty01 concurrent\n");
		exit(0);
	}
	close(fd);

}

