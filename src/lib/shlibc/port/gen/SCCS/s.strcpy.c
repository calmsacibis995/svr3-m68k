h44566
s 00026/00000/00000
d D 1.1 86/07/31 10:25:44 fnf 1 0
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

#ident	"@(#)libc-port:gen/strcpy.c	1.4"
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
/*
 * Copy string s2 to s1.  s1 must be large enough.
 * return s1
 */

char *
strcpy(s1, s2)
register char *s1, *s2;
{
	register char *os1;

	os1 = s1;
	while(*s1++ = *s2++)
		;
	return(os1);
}
E 1
