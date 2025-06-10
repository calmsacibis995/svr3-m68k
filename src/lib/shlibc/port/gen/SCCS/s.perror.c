h64329
s 00036/00000/00000
d D 1.1 86/07/31 10:25:35 fnf 1 0
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

#ident	"@(#)libc-port:gen/perror.c	1.7"
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
/*
 * Print the error indicated
 * in the cerror cell.
 */

extern int errno, sys_nerr, strlen(), write();
extern char *sys_errlist[];

void
perror(s)
char	*s;
{
	register char *c;
	register int n;

	c = "Unknown error";
	if(errno < sys_nerr)
		c = sys_errlist[errno];
	n = strlen(s);
	if(n) {
		(void) write(2, s, (unsigned)n);
		(void) write(2, ": ", 2);
	}
	(void) write(2, c, (unsigned)strlen(c));
	(void) write(2, "\n", 1);
}
E 1
