h23631
s 00015/00000/00000
d D 1.1 86/07/31 12:00:11 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
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
E 1
