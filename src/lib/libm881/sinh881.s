#	@(#)sinh881.s	6.1	


	file	"sinh881.s"

# unsigned int
# sinh881(x,y)
# double x;
# double *y;
# {
	set	F%1,-4
	global	sinh881
sinh881:
	link.l	%fp,&F%1
# unsigned register int exc;		/* imagine exc as being %d0 */
# exc = fsinh(x,&y);
	fsinh.d (8,%fp),%fp0
	fmovm.l	%status,%d0		# contains any exception or
					# condition code info
	fmov.d	%fp0,([16.w,%fp])
	fmovm.l	%status,%d1		# may contain overflow info
	or.l	%d1,%d0
# return (exc);				/* it is already in %d0 */
# }
	unlk	%fp
	rts


# unsigned int
# cosh881(x,y)
# double x;
# double *y;
# {
	set	F%2,-4
	global	cosh881
cosh881:
	link.l	%fp,&F%2
# unsigned register int exc;		/* imagine exc as being %d0 */
# exc = fcosh(x,&y);
	fcosh.d (8,%fp),%fp0
	fmovm.l	%status,%d0		# contains any exception or
					# condition code info
	fmov.d	%fp0,([16.w,%fp])
	fmovm.l	%status,%d1		# may contain overflow info
	or.l	%d1,%d0
# return (exc);				/* it is already in %d0 */
# }
	unlk	%fp
	rts
