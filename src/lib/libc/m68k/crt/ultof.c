static char SCCSID[] = "@(#)ultof.c	6.1	";
/*
 *	unsigned long to float conversion
 */

#include <sys/fp.h>

#define MASK 0xfe000000L
#define RBIT (HIDDENBIT << 1)

fp
ultof (x)
	unsigned long x;
{
	fp result;
	unsigned long frac;
	int exp;

	/* check for zero argument */
	if (x == 0)
		return zero;
	
	/* initial conditions */
	frac = x;
	exp = EXPOFFSET + FRACSIZE + 2;

	/* shift right if necessary */
	while (frac & MASK) {
		frac >>= 1;
		exp++;
	}

	/* shift left if necessary */
	while ((frac & RBIT) == 0) {
		frac <<= 1;
		exp--;
	}

	/* round and renormalize */
	frac = (frac + 1) >> 1;
	if ((frac & HIDDENBIT) == 0) {
		exp++;
		frac >>= 1;
	}

	/* store result */
	result.sign = 0;
	result.exp = exp;
	result.frac = frac & ~HIDDENBIT;
	return result;
}
