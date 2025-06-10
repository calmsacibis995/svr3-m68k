h23598
s 00046/00000/00000
d D 1.1 86/07/31 12:00:17 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
static char SCCSID[] = "@(#)ldexp.c	6.1	";
/*
 *	ldexp - combine a fraction and exponent
 *
 *	fp ldexp(value, exp)
 *	fp value;
 *	int exp;
 *	
 *	Ldexp returns value * 2**exp, if that result is in range.
 *	If underflow occurs, it returns zero.  If overflow occurs,
 *	it returns a value of appropriate sign and largest single-
 *	precision magnitude.  In case of underflow or overflow,
 *	the external int "errno" is set to ERANGE.  Note that errno
 *	is not modified if no error occurs, so if you intend to test
 *	it after you use ldexp, you had better set it to something
 *	other than ERANGE first (zero is a reasonable value to use).
 */

#include "fp.h"
#include <errno.h>

fp
ldexp (value, exp)
	fp value;
	int exp;
{
	/* check for zero argument */
	if (value.exp == 0)
		return zero;
	
	exp += value.exp;

	/* range check */
	if (exp > MAXEXP) {
		errno = ERANGE;
		exp = MAXEXP;
		value.frac = ~0;
	} else if (exp <= 0) {
		errno = ERANGE;
		return zero;
	}
	
	value.exp = exp;

	return value;
}
E 1
