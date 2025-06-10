h50354
s 00038/00000/00000
d D 1.1 86/07/31 12:00:16 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
static char SCCSID[] = "@(#)ftol.c	6.1	";
/*
 *	floating-point to long conversion
 */

#include <sys/fp.h>

long
ftol (x)
	fp x;
{
	unsigned long l;
	register int exp, shift;


	/* extract exponent, test for zero */
	exp = x.exp;
	if (exp == 0)
		return 0L;
	
	/* extract fraction, restore hidden bit */
	l = x.frac | HIDDENBIT;

	/* calculate how far to shift */
	shift = exp - EXPOFFSET - FRACSIZE - 1;

	/* shift in the proper direction */
	if (shift > 0)
		l <<= shift;
	else
		l >>= -shift;

	/* compensate for sign */
	if (x.sign)
		l = -l;
	
	return l;
}
E 1
