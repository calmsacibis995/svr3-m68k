h29268
s 00021/00000/00000
d D 1.1 86/07/31 12:00:20 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
static char SCCSID[] = "@(#)ltof.c	6.1	";
/*
 *	long signed integer to float
 */

#include <sys/fp.h>

fp
ltof (x)
	register long x;
{
	fp result;

	if (x >= 0)
		return ultof ((unsigned long) x);
	
	result = ultof ((unsigned long) -x);
	result.sign = 1;

	return result;
}
E 1
