h51674
s 00028/00000/00000
d D 1.1 86/07/31 10:25:47 fnf 1 0
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

#ident	"@(#)libc-port:gen/strncmp.c	1.7"
/*LINTLIBRARY*/
/*
 * Compare strings (at most n bytes)
 *	returns: s1>s2; >0  s1==s2; 0  s1<s2; <0
 */
#include <sys/types.h>

int
strncmp(s1, s2, n)
register char *s1, *s2;
register size_t n;
{
	n++;
	if(s1 == s2)
		return(0);
	while(--n > 0 && *s1 == *s2++)
		if(*s1++ == '\0')
			return(0);
	return((n == 0)? 0: (*s1 - *--s2));
}
E 1
