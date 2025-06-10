h30049
s 00038/00000/00000
d D 1.1 86/07/31 10:25:32 fnf 1 0
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

#ident	"@(#)libc-port:gen/modf.c	1.10"
/*LINTLIBRARY*/
/*
 * modf(value, iptr) returns the signed fractional part of value
 * and stores the integer part indirectly through iptr.
 *
 */

#include <nan.h>
#include <values.h>

double
modf(value, iptr)
double value; /* don't declare register, because of KILLNaN! */
register double *iptr;
{
	register double absvalue;

	KILLNaN(value); /* raise exception on Not-a-Number (3b only) */
	if ((absvalue = (value >= 0.0) ? value : -value) >= MAXPOWTWO)
		*iptr = value; /* it must be an integer */
	else {
		*iptr = absvalue + MAXPOWTWO; /* shift fraction off right */
		*iptr -= MAXPOWTWO; /* shift back without fraction */
		while (*iptr > absvalue) /* above arithmetic might round */
			*iptr -= 1.0; /* test again just to be sure */
		if (value < 0.0)
			*iptr = -*iptr;
	}
	return (value - *iptr); /* signed fractional part */
}
E 1
