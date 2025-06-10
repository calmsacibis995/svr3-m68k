static char SCCSID[] = "@(#)submag.c	6.1	";
/*
 *	subtract magnitude of y from magnitude of x.
 *	result has sign of x if |y| <= |x|, opposite
 *	sign otherwise.  sign of y is ignored.
 */

#include <sys/fp.h>

fp
fp_submag (x, y)
	fp x, y;
{
	fp result;
	register int lexp, sexp;
	register long lfrac, sfrac, rfrac;
	register int shift;
	register long t;

	result.sign = x.sign;

	/*
	 *	put exponent, fraction of large operand into lexp, lfrac
	 *	and exponent, fraction of small operand into sexp, sfrac
	 *	flip result sign if necessary
	 */
	if (x.exp > y.exp || x.exp == y.exp && x.frac >= y.frac) {
		lexp = x.exp;
		lfrac = x.frac;
		sexp = y.exp;
		sfrac = y.frac;
	} else {
		lexp = y.exp;
		lfrac = y.frac;
		sexp = x.exp;
		sfrac = x.frac;
		result.sign ^= 1;
	}

	/* If larger number is infinity, return infinity */
	if (lexp == INFEXP) {
		result.exp = infinity.exp;
		result.frac = infinity.frac;
		return result;
	}

	/* install hidden bit in both fractions and create guard bits */
	lfrac = (lfrac | HIDDENBIT) << GBITS;
	sfrac = (sfrac | HIDDENBIT) << GBITS;

	/* difference between exponents is how many bits to shift */
	shift = lexp - sexp;

	/*
	 *	if the smaller operand is zero
	 *	or the exponent difference is too large,
	 *	return the larger operand
	 */
	if (shift > FRACSIZE + 2 || sexp == 0) {
		result.exp = lexp;
		result.frac = (lfrac >> GBITS) & ~HIDDENBIT;
		return result;
	}

	/* the actual subtraction, with sticky right shift */
	t = sfrac >> shift;
	rfrac = lfrac - t;
	if (sfrac != t << shift)
		rfrac--;

	/* if the result is zero, return true zero */
	if (rfrac == 0)
		return zero;
	
	/* result nonzero, normalize */
	while ((rfrac & NORMMASK) == 0) {
		lexp--;
		rfrac <<= 1;
	}

	/* round */
	if (rfrac & (1 << (GBITS - 1))) {
		rfrac += 1 << GBITS;
	
		/* we may need to renormalize (no more than once) */
		if (rfrac & CARRYBIT) {
			lexp++;
			rfrac >>= 1;
		}
	}

	/* check for underflow */
	if (lexp <= 0)
		return zero;

	/* store final result */
	result.exp = lexp;
	result.frac = (rfrac >> GBITS) & ~HIDDENBIT;

	return result;
}
