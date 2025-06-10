h21573
s 00059/00000/00000
d D 1.1 86/07/31 12:00:23 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
static char SCCSID[] = "@(#)modf.c	6.1	";
/*
 *	modf - split a float into integer and fractional part
 *	*iptr gets integer part, return fractional part
 */

#include "fp.h"

fp
modf (value, iptr)
	fp value, *iptr;
{
	int exp;
	unsigned long frac, mask;

	/* extract exponent */
	exp = value.exp;

	/* is it all fraction? */
	if (exp <= EXPOFFSET) {
		*iptr = zero;
		return value;
	}

	/* initialize the integer and fractional parts */
	*iptr = value;
	frac = value.frac | HIDDENBIT;

	/* is it all integer */
	if (exp > EXPOFFSET + FRACSIZE) {
		return zero;
	}

	/* create the mask to separate integer and fraction */
	mask = -1L << (EXPOFFSET + FRACSIZE + 1 - exp);

	/* store the integer part */
	iptr->frac &= mask;

	/* build and normalize the fractional part */
	frac &= ~mask;
	if (frac == 0)
		return zero;
	
	while ((frac & HIDDENBIT) == 0) {
		frac <<= 1;
		exp--;
	}

	/* underflow? */
	if (exp <= 0)
		return zero;

	/* build the fractional part */
	value.exp = exp;
	value.frac = frac & ~HIDDENBIT;

	return value;
}
E 1
