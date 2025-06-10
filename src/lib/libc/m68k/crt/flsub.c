static char SCCSID[] = "@(#)flsub.c	6.1	";
/* subtract two floating-point numbers */

#include <sys/fp.h>

fp
flsub (x, y)
	fp x, y;
{
	if (x.sign != y.sign)
		return fp_addmag (x, y);
	else
		return fp_submag (x, y);
}
