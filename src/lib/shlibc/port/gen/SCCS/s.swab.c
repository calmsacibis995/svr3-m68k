h47375
s 00026/00000/00000
d D 1.1 86/07/31 10:26:02 fnf 1 0
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

#ident	"@(#)libc-port:gen/swab.c	1.4"
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
/*
 * Swap bytes in 16-bit [half-]words
 * for going between the 11 and the interdata
 */

void
swab(pf, pt, n)
register short *pf, *pt;
register int n;
{
	n /= 2;
	while(--n >= 0) {
		*pt++ = (*pf << 8) + ((*pf >> 8) & 0377);
		pf++;
	}
}
E 1
