h59781
s 00041/00000/00000
d D 1.1 86/07/31 10:25:21 fnf 1 0
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

#ident	"@(#)libc-port:gen/lfind.c	1.4"
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
/*
 * Linear search algorithm, generalized from Knuth (6.1) Algorithm Q.
 *
 * This version no longer has anything to do with Knuth's Algorithm Q,
 * which first copies the new element into the table, then looks for it.
 * The assumption there was that the cost of checking for the end of the
 * table before each comparison outweighed the cost of the comparison, which
 * isn't true when an arbitrary comparison function must be called and when the
 * copy itself takes a significant number of cycles.
 * Actually, it has now reverted to Algorithm S, which is "simpler."
 */

#include <stdio.h>
typedef char *POINTER;
extern POINTER memcpy();

POINTER
lfind(key, base, nelp, width, compar)
register POINTER key;		/* Key to be located */
register POINTER base;		/* Beginning of table */
unsigned *nelp;			/* Pointer to current table size */
register unsigned width;	/* Width of an element (bytes) */
int (*compar)();		/* Comparison function */
{
	register POINTER next = base + *nelp * width;	/* End of table */

	for ( ; base < next; base += width)
		if ((*compar)(key, base) == 0)
			return (base);	/* Key found */
	return (NULL);
}
E 1
