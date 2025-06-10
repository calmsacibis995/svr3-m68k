h48592
s 00023/00000/00000
d D 1.1 86/07/31 10:25:25 fnf 1 0
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

#ident	"@(#)libc-port:gen/memchr.c	1.3"
/*LINTLIBRARY*/
/*
 * Return the ptr in sp at which the character c appears;
 *   NULL if not found in n chars; don't stop at \0.
 */
char *
memchr(sp, c, n)
register char *sp, c;
register int n;
{
	while (--n >= 0)
		if (*sp++ == c)
			return (--sp);
	return (0);
}
E 1
