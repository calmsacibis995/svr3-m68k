h24599
s 00014/00000/00000
d D 1.1 86/07/31 12:00:08 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
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
E 1
