h60299
s 00070/00000/00000
d D 1.1 86/07/31 11:42:53 fnf 1 0
c Initial copy from 5.3 distribution for 3b2.
e
u
U
t
T
I 1
/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:stdio/getpass.c	1.10"
/*	3.0 SID #	1.4	*/
/*LINTLIBRARY*/
#include <stdio.h>
#include <signal.h>
#include <termio.h>

extern void setbuf();
extern FILE *fopen();
extern int fclose(), fprintf(), findiop();
extern int kill(), ioctl(), getpid();
static int intrupt;

#define	MAXPASSWD	8	/* max significant characters in password */

static char dev_tty[] = "/dev/tty";


char *
getpass(prompt)
char	*prompt;
{
	struct termio ttyb;
	unsigned short flags;
	register char *p;
	register int c;
	FILE	*fi;
	static char pbuf[ MAXPASSWD + 1 ];
	int	(*sig)(), catch();

	if((fi = fopen(dev_tty,"r")) == NULL)
		return((char*)NULL);
	setbuf(fi, (char*)NULL);
	sig = signal(SIGINT, catch);
	intrupt = 0;
	(void) ioctl(fileno(fi), TCGETA, &ttyb);
	flags = ttyb.c_lflag;
	ttyb.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL);
	(void) ioctl(fileno(fi), TCSETAF, &ttyb);
	(void) fputs(prompt, stderr);
	p = pbuf;
	while( !intrupt  &&
		(c = getc(fi)) != '\n'  &&  c != '\r'  &&  c != EOF ) {
		if(p < &pbuf[ MAXPASSWD ])
			*p++ = c;
	}
	*p = '\0';
	ttyb.c_lflag = flags;
	(void) ioctl(fileno(fi), TCSETAW, &ttyb);
	(void) putc('\n', stderr);
	(void) signal(SIGINT, sig);
	if(fi != stdin)
		(void) fclose(fi);
	if(intrupt)
		(void) kill(getpid(), SIGINT);
	return(pbuf);
}

static int
catch()
{
	++intrupt;
}
E 1
