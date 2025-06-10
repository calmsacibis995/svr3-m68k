h27768
s 00021/00000/00000
d D 1.1 86/07/31 12:00:16 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
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
E 1
