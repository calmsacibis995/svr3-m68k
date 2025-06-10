h14628
s 00078/00000/00000
d D 1.1 86/07/31 10:25:19 fnf 1 0
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

#ident	"@(#)libc-port:gen/l3.c	1.9"
/*LINTLIBRARY*/
/*
 * Convert longs to and from 3-byte disk addresses
 */

void
ltol3(cp, lp, n)
char	*cp;
long	*lp;
int	n;
{
	register i;
	register char *a, *b;

	a = cp;
	b = (char *)lp;
	for(i=0; i < n; ++i) {
#if interdata || u370 || u3b || M32
		b++;
		*a++ = *b++;
		*a++ = *b++;
		*a++ = *b++;
#endif
#if vax
		*a++ = *b++;
		*a++ = *b++;
		*a++ = *b++;
		b++;
#endif
#if pdp11
		*a++ = *b++;
		b++;
		*a++ = *b++;
		*a++ = *b++;
#endif
	}
}

void
l3tol(lp, cp, n)
long	*lp;
char	*cp;
int	n;
{
	register i;
	register char *a, *b;

	a = (char *)lp;
	b = cp;
	for(i=0; i < n; ++i) {
#if interdata || u370 || u3b || M32
		*a++ = 0;
		*a++ = *b++;
		*a++ = *b++;
		*a++ = *b++;
#endif
#if vax
		*a++ = *b++;
		*a++ = *b++;
		*a++ = *b++;
		*a++ = 0;
#endif
#if pdp11
		*a++ = *b++;
		*a++ = 0;
		*a++ = *b++;
		*a++ = *b++;
#endif
	}
}
E 1
