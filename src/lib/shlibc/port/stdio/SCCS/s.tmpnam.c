h60168
s 00036/00000/00000
d D 1.1 86/07/31 11:43:14 fnf 1 0
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

#ident	"@(#)libc-port:stdio/tmpnam.c	1.8"
/*LINTLIBRARY*/
#include <stdio.h>

extern char *mktemp(), *strcpy(), *strcat();
static char str[L_tmpnam], seed[] = { 'a', 'a', 'a', '\0' };

static char Xs[] = "XXXXXX";

char *
tmpnam(s)
char	*s;
{
	register char *p, *q;

	p = (s == NULL)? str: s;
	(void) strcpy(p, P_tmpdir);
	(void) strcat(p, seed);
	(void) strcat(p, Xs);

	q = seed;
	while(*q == 'z')
		*q++ = 'a';
	if (*q != '\0')
		++*q;

	(void) mktemp(p);
	return(p);
}
E 1
