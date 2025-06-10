static char SCCSID[] = "@(#)fldiv.c	6.1	";
/*
 *	floating-point divide
 */

#include <sys/fp.h>

fp
fldiv (x, y)
	fp x, y;
{
	fp result;
	int exp;
	register i;
	long z, q, r, xfrac, yfrac;

	/* if divisor is zero, return infinity with proper sign */
	if (y.exp == 0) {
		result = infinity;
		result.sign = x.sign;
		return result;
	}

	/* if dividend is zero, return zero */
	if (x.exp == 0)
		return zero;

	/* if dividend is infinity, return infinity */
	if (x.exp == INFEXP) {
		result = x;
		result.sign ^= y.sign;
		return result;
	}
	
	/* calculate result exponent */
	exp = x.exp - y.exp + EXPOFFSET + FRACSIZE + 2;

	/* extract the true fractions */
	xfrac = x.frac | HIDDENBIT;
	yfrac = y.frac | HIDDENBIT;

	/* divide, by repeated subtraction (ugh) */
	q = 0;
	do {
		q <<= 1;
		if (xfrac >= yfrac) {
			q++;
			xfrac -= yfrac;
		}
		xfrac <<= 1;
		exp--;
	} while ((q & HIDDENBIT) == 0);

	/* round, perhaps renormalize */
	if (xfrac >= yfrac) {
		q++;
		if ((q & HIDDENBIT) == 0) {
			q >>= 1;
			exp++;
		}
	}

	/* underflow? */
	if (exp < 1)
		return zero;

	/* store result or overflow indication */
	if (exp > MAXEXP)
		result = infinity;
	else {
		result.exp = exp;
		result.frac = q & ~HIDDENBIT;
	}

	/* result sign */
	result.sign = x.sign ^ y.sign;

	return result;
}
