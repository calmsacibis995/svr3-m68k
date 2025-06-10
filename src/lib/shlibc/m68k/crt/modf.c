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
