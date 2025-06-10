h47821
s 00036/00000/00000
d D 1.1 86/07/31 12:00:31 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
static char SCCSID[] = "@(#)uitof.c	6.1	";
/*
 *	unsigned integer to float
 */

#include <sys/fp.h>

fp
uitof (x)
	register unsigned x;
{
	fp result;
	long frac;
	register int exp;
	
	/* converting zero? */
	if (x == 0)
		return zero;
	
	/* create an unnormalized fraction and exponent */
	frac = x;
	exp = EXPOFFSET + FRACSIZE + 1;

	/* normalize */
	while ((frac & HIDDENBIT) == 0) {
		frac <<= 1;
		exp--;
	}

	/* store the result */
	result.sign = 0;
	result.exp = exp;
	result.frac = frac & ~HIDDENBIT;

	return result;
}
E 1
