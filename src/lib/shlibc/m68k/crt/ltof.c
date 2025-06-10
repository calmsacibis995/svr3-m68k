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
