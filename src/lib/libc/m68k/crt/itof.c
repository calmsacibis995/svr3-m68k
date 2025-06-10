static char SCCSID[] = "@(#)itof.c	6.1	";
/*
 *	signed integer to float
 */

#include <sys/fp.h>

fp
itof (x)
	register int x;
{
	fp result;

	if (x >= 0)
		return uitof ((unsigned) x);
	
	result = uitof ((unsigned) -x);
	result.sign = 1;

	return result;
}
