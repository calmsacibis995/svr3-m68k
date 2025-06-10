#	@(#)sqrt881.s	6.1	


	file	"sqrt881.s"

# unsigned int
# sqrt881(x,y)
# double x;
# double *y;
# {
	set	F%1,-4
	global	sqrt881
sqrt881:
	link.l	%fp,&F%1
# unsigned register int exc;		/* imagine exc as being %d0 */
# exc = fsqrt(x,&y);
	fsqrt.d (8,%fp),%fp0
	fmovm.l	%status,%d0		# contains any exception or
					# condition code info
	fmov.d	%fp0,([16.w,%fp])
	fmovm.l	%status,%d1		# may contain overflow info
	or.l	%d1,%d0
# return (exc);				/* it is already in %d0 */
# }
L%12:
	unlk	%fp
	rts
