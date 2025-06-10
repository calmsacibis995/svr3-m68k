#	@(#)tanh881.s	6.1	



	file	"tanh881.s"

# unsigned int
# tanh881(x,y)
# double x;
# double *y;
# {
	set	F%1,-4
	global	tanh881
tanh881:
	link.l	%fp,&F%1
# unsigned register int exc;		/* imagine exc as being %d0 */
# exc = ftanh(x,&y);
	ftanh.d (8,%fp),%fp0
	fmovm.l	%status,%d0		# contains any exception or
					# condition code info
	fmov.d	%fp0,([16.w,%fp])
# return (exc);				/* it is already in %d0 */
# }
L%12:
	unlk	%fp
	rts
