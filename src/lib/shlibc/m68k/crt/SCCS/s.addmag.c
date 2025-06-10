h65363
s 00083/00000/00000
d D 1.1 86/07/31 11:59:56 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
static char SCCSID[] = "@(#)addmag.c	6.1	";
/*
 *	add magnitudes of values addressed by x and y.
 *	sign of y is ignored, result sign = sign of x.
 */

#include <sys/fp.h>

fp
fp_addmag (x, y)
	register fp x, y;
{
	fp result;
	register int lexp, sexp;
	register long lfrac, sfrac, rfrac;
	register int shift;

	result.sign = x.sign;

	/*
	 *	put large exponent, fraction into lexp, lfrac
	 *	and small exponent, fraction into sexp, sfrac
	 */
	if (x.exp > y.exp) {
		lexp = x.exp;
		lfrac = x.frac;
		sexp = y.exp;
		sfrac = y.frac;
	} else {
		lexp = y.exp;
		lfrac = y.frac;
		sexp = x.exp;
		sfrac = x.frac;
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

	/* the actual addition */
	rfrac = lfrac + (sfrac >> shift);

	/* check for a carry during addition */
	if (rfrac & CARRYBIT) {
		lexp++;
		rfrac >>= 1;
	}

	/* rounding */
	if (rfrac & (1 << (GBITS - 1))) {
		rfrac += 1 << GBITS;
		if (rfrac & CARRYBIT) {
			lexp++;
			rfrac >>= 1;
		}
	}

	/* overflow check */
	if (lexp > MAXEXP) {
		lexp = infinity.exp;
		rfrac = infinity.frac << GBITS;
	}

	/* store final result */
	result.exp = lexp;
	result.frac = (rfrac >> GBITS) & ~HIDDENBIT;

	return result;
}
E 1
