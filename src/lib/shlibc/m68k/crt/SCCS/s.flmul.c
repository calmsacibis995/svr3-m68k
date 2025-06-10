h51397
s 00082/00000/00000
d D 1.1 86/07/31 12:00:10 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
static char SCCSID[] = "@(#)flmul.c	6.1	";
/*
 *	floating-point multiply
 */

#include <sys/fp.h>

fp
flmul (x, y)
	fp x, y;
{
	fp result;
	int exp;
	long zh, zm, zl, xfrac, yfrac;
	short xh, xl, yh, yl;

	/* if either operand is infinity, return infinity */
	if (x.exp == INFEXP || y.exp == INFEXP) {
		result = infinity;
		result.sign = x.sign ^ y.sign;
		return result;
	}
	
	/* if either operand is zero, return zero */
	if (x.exp == 0 || y.exp == 0)
		return zero;

	/* calculate result exponent */
	exp = x.exp + y.exp - EXPOFFSET;

	/* extract the true fractions */
	xfrac = x.frac | HIDDENBIT;
	yfrac = y.frac | HIDDENBIT;

	/* split the fractions into pieces */
	xh = hi (xfrac);
	xl = lo (xfrac);
	yh = hi (yfrac);
	yl = lo (yfrac);

	/* multiply the pieces, but don't merge carries yet */
	zh = lmul (xh, yh);
	zm = lmul (xh, yl) + lmul (xl, yh);
	zl = lmul (xl, yl);

	/* propagate carries */
	zm += hi (zl);
	zh += hi (zm);

	/* normalize if necessary */
	while ((zh & HIDDENBIT) == 0) {
		zh = (zh << 1) | hibit (zm);
		zm <<= 1;
		exp--;
	}

	/* round, perhaps renormalize */
	if (hibit (zm)) {
		zh++;
		if ((zh & HIDDENBIT) == 0) {
			zh >>= 1;
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
		result.frac = zh & ~HIDDENBIT;
	}

	/* result sign */
	result.sign = x.sign ^ y.sign;

	return result;
}
E 1
