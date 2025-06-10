h00146
s 00037/00000/00000
d D 1.1 86/07/31 11:42:34 fnf 1 0
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

#ident	"@(#)libc-port:stdio/cuserid.c	1.8"
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
#include <stdio.h>
#include <pwd.h>

extern char *strcpy(), *getlogin();
extern int getuid();
extern struct passwd *getpwuid();
static char res[L_cuserid];

char *
cuserid(s)
char	*s;
{
	register struct passwd *pw;
	register char *p;

	if (s == NULL)
		s = res;
	p = getlogin();
	if (p != NULL)
		return (strcpy(s, p));
	pw = getpwuid(getuid());
	endpwent();
	if (pw != NULL)
		return (strcpy(s, pw->pw_name));
	*s = '\0';
	return (NULL);
}
E 1
