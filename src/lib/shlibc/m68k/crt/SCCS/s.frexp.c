h32376
s 00026/00000/00000
d D 1.1 86/07/31 12:00:15 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
static char SCCSID[] = "@(#)frexp.c	6.1	";
/*
 *	frexp - split a float into a fraction and exponent
 */

#include "fp.h"

fp
frexp (value, eptr)
	fp value;
	int *eptr;
{
	register exp;

	exp = value.exp;

	if (exp == 0) {
		*eptr = 0;
		return zero;
	}

	*eptr = exp - EXPOFFSET;
	value.exp = EXPOFFSET;

	return value;
}
E 1
