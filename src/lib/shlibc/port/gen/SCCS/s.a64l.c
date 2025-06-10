h55163
s 00034/00000/00000
d D 1.1 86/07/31 10:24:51 fnf 1 0
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

#ident	"@(#)libc-port:gen/a64l.c	1.10"
/*	3.0 SID #	1.3	*/
/*LINTLIBRARY*/
/*
 * convert base 64 ascii to long int
 * char set is [./0-9A-Za-z]
 *
 */

#define BITSPERCHAR	6 /* to hold entire character set */

long
a64l(s)
register char *s;
{
	register int i, c;
	long lg = 0;

	for (i = 0; (c = *s++) != '\0'; i += BITSPERCHAR) {
		if (c > 'Z')
			c -= 'a' - 'Z' - 1;
		if (c > '9')
			c -= 'A' - '9' - 1;
		lg |= (long)(c - ('0' - 2)) << i;
	}
	return (lg);
}
E 1
