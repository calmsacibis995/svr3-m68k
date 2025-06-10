static char SCCSID[] = "@(#)flneg.c	6.1	";
/* change the sign of a floating-point number */

#include <sys/fp.h>

fp
flneg (x)
	fp x;
{
	if (x.sign)
		x.sign = 0;
	else if (x.exp)
		x.sign = 1;
	return x;
}
