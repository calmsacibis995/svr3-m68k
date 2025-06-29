h28279
s 00038/00000/00000
d D 1.1 86/07/31 12:17:00 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
/*	@(#)modf.c	7.2		*/
/*	@(#)modf.c	6.1 	*/
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

#ifdef m68k
/* Support for IEEE std. infinity's and NaN's */
	KILLNaN(value); /* raise exception on Not-a-Number */
	CHNGinf(value); /* detect infinities and turn them into large #'s */
#else
	KILLNaN(value); /* raise exception on Not-a-Number (3b only) */
#endif
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
