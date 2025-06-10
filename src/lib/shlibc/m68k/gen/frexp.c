/*	@(#)frexp.c	7.2		*/
/*	@(#)frexp.c	6.1 	*/
/*LINTLIBRARY*/
/*
 * frexp(value, eptr)
 * returns a double x such that x = 0 or 0.5 <= |x| < 1.0
 * and stores an integer n such that value = x * 2 ** n
 * indirectly through eptr.
 *
 */
#ifdef m68k
#include <values.h>
#endif
#include <nan.h>

double
frexp(value, eptr)
double value; /* don't declare register, because of KILLNan! */
register int *eptr;
{
	register double absvalue;

#ifdef m68k
/* Support for IEEE std. infinity's and NaN's */
	KILLNaN(value); /* raise exception on Not-a-Number */
	CHNGinf(value); /* detect infinities and turn them into large #'s */
#else
	KILLNaN(value); /* raise exception on Not-a-Number (3b only) */
#endif
	*eptr = 0;
	if (value == 0.0) /* nothing to do for zero */
		return (value);
	absvalue = (value > 0.0) ? value : -value;
	for ( ; absvalue >= 1.0; absvalue *= 0.5)
		++*eptr;
	for ( ; absvalue < 0.5; absvalue += absvalue)
		--*eptr;
	return (value > 0.0 ? absvalue : -absvalue);
}
