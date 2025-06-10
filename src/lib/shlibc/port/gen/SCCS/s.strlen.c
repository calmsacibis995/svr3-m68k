h43786
s 00025/00000/00000
d D 1.1 86/07/31 10:25:46 fnf 1 0
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

#ident	"@(#)libc-port:gen/strlen.c	1.5"
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
/*
 * Returns the number of
 * non-NULL bytes in string argument.
 */

int
strlen(s)
register char *s;
{
	register char *s0 = s + 1;

	while (*s++ != '\0')
		;
	return (s - s0);
}
E 1
