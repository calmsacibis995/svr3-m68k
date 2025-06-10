static char SCCSID[] = "@(#)fladd.c	6.1	";
/* add two floating-point numbers */

#include <sys/fp.h>

fp
fladd (x, y)
	fp x, y;
{
	if (x.sign == y.sign)
		return fp_addmag (x, y);
	else
		return fp_submag (x, y);
}
