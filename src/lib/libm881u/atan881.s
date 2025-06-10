#	@(#)atan881.s	6.1	



	file	"atan881.s"

# unsigned int
# atan881(x,y)
# double x;
# double *y;
# {
	set	F%1,-4
	global	atan881
atan881:
	link.l	%fp,&F%1
# unsigned register int exc;		/* imagine exc as being %d0 */
# exc = fatan(x,&y);
	fatan.d (8,%fp),%fp0
	fmovm.l	%status,%d0		# contains any exception or
					# condition code info
	fmov.d	%fp0,([16.w,%fp])
# return (exc);				/* it is already in %d0 */
# }
L%12:
	unlk	%fp
	rts
