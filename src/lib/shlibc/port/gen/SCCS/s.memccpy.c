h56857
s 00024/00000/00000
d D 1.1 86/07/31 10:25:24 fnf 1 0
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

#ident	"@(#)libc-port:gen/memccpy.c	1.3"
/*LINTLIBRARY*/
/*
 * Copy s2 to s1, stopping if character c is copied. Copy no more than n bytes.
 * Return a pointer to the byte after character c in the copy,
 * or NULL if c is not found in the first n bytes.
 */
char *
memccpy(s1, s2, c, n)
register char *s1, *s2;
register int c, n;
{
	while (--n >= 0)
		if ((*s1++ = *s2++) == c)
			return (s1);
	return (0);
}
E 1
