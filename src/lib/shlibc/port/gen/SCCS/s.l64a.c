h33151
s 00052/00000/00000
d D 1.1 86/07/31 10:25:20 fnf 1 0
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

#ident	"@(#)libc-port:gen/l64a.c	1.10"
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
/*
 * convert long int to base 64 ascii
 * char set is [./0-9A-Za-z]
 * two's complement negatives are assumed,
 * but no assumptions are made about sign propagation on right shift
 *
 */

#include <values.h>

#define BITSPERCHAR	6 /* to hold entire character set */
#define BITSPERLONG	(BITSPERBYTE * sizeof(long))
#define NMAX	((BITSPERLONG + BITSPERCHAR - 1)/BITSPERCHAR)
#define SIGN	(-(1L << (BITSPERLONG - BITSPERCHAR - 1)))
#define CHARMASK	((1 << BITSPERCHAR) - 1)
#define WORDMASK	((1L << ((NMAX - 1) * BITSPERCHAR)) - 1)

static char buf[NMAX + 1];

char *
l64a(lg)
register long lg;
{
	register char *s = buf;

	while (lg != 0) {

		register int c = ((int)lg & CHARMASK) + ('0' - 2);

		if (c > '9')
			c += 'A' - '9' - 1;
		if (c > 'Z')
			c += 'a' - 'Z' - 1;
		*s++ = c;
		/* fill high-order CHAR if negative */
		/* but suppress sign propagation */
		lg = ((lg < 0) ? (lg >> BITSPERCHAR) | SIGN :
			lg >> BITSPERCHAR) & WORDMASK;
	}
	*s = '\0';
	return (buf);
}
E 1
