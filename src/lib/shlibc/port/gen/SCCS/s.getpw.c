h09306
s 00056/00000/00000
d D 1.1 86/07/31 10:25:14 fnf 1 0
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

#ident	"@(#)libc-port:gen/getpw.c	1.5"
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
#include <stdio.h>
#include <ctype.h>

extern void rewind();
extern FILE *fopen();

static FILE *pwf;

int
getpw(uid, buf)
int	uid;
char	buf[];
{
	static char etc_pass[] = "/etc/passwd";
	register n, c;
	register char *bp;

	if(pwf == 0)
		pwf = fopen(etc_pass, "r");
	if(pwf == NULL)
		return(1);
	rewind(pwf);

	while(1) {
		bp = buf;
		while((c=getc(pwf)) != '\n') {
			if(c == EOF)
				return(1);
			*bp++ = c;
		}
		*bp = '\0';
		bp = buf;
		n = 3;
		while(--n)
			while((c = *bp++) != ':')
				if(c == '\n')
					return(1);
		while((c = *bp++) != ':')
			if(isdigit(c))
				n = n*10+c-'0';
			else
				continue;
		if(n == uid)
			return(0);
	}
}
E 1
