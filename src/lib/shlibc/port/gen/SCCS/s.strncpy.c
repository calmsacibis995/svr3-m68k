h54232
s 00031/00000/00000
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

#ident	"@(#)libc-port:gen/strncpy.c	1.8"
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
/*
 * Copy s2 to s1, truncating or null-padding to always copy n bytes
 * return s1
 */
#include <sys/types.h>

char *
strncpy(s1, s2, n)
register char *s1, *s2;
register size_t n;
{
	register char *os1 = s1;

	n++;				
	while ((--n > 0) &&  ((*s1++ = *s2++) != '\0'))
		;
	if (n > 0)
		while (--n > 0)
			*s1++ = '\0';
	return (os1);
}
E 1
