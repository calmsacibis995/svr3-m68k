h43538
s 00025/00000/00000
d D 1.1 86/07/31 10:25:43 fnf 1 0
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

#ident	"@(#)libc-port:gen/strcmp.c	1.4"
/*LINTLIBRARY*/
/*
 * Compare strings:  s1>s2: >0  s1==s2: 0  s1<s2: <0
 */

int
strcmp(s1, s2)
register char *s1, *s2;
{

	if(s1 == s2)
		return(0);
	while(*s1 == *s2++)
		if(*s1++ == '\0')
			return(0);
	return(*s1 - *--s2);
}
E 1
