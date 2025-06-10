h34047
s 00027/00000/00000
d D 1.1 86/07/31 12:00:08 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
static char SCCSID[] = "@(#)flcmp.c	7.2	";
/*
 *	floating-point comparison -- specific to 68000
 */

int
flcmp (x, y)
	register long x, y;
{
	register int r;

	r = 0;

	if ((unsigned)x == 0x80000000)
		x = 0; /* convert -0 to +0 */
	if ((unsigned)y == 0x80000000)
		y = 0; /* convert -0 to +0 */
	if (x < y)
		r = -1;
	else if (x > y)
		r = 1;

	if (x < 0 && y < 0)
		r = -r;

	return r;
}
E 1
