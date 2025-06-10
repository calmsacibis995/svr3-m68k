h25206
s 00014/00000/00000
d D 1.1 86/07/31 12:00:11 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
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
E 1
