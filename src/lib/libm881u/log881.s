#	@(#)log881.s	7.2	
#	@(#)log881.s	7.1	


	file	"log881.s"

# unsigned int
# log881(x,y)
# double x;
# double *y;
# {
	set	F%1,-4
	global	log881
log881:
	link.l	%fp,&F%1
# unsigned register int exc;		/* imagine exc as being %d0 */
# exc = flog(x,*y);
	flog10.d (8,%fp),%fp0
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
# logn881(x,y)
# double x;
# double *y;
# {
	set	F%2,-4
	global	logn881
logn881:
	link.l	%fp,&F%2
# unsigned register int exc;		/* imagine exc as being %d0 */
# exc = flogn(x,*y);
	fmov.d	(8,%fp),%fp1
	fsub.w	&1,%fp1
	flognp1.x %fp1,%fp0
	fmovm.l	%status,%d0		# contains any exception or
					# condition code info
	fmov.d	%fp0,([16.w,%fp])
	fmovm.l	%status,%d1		# may contain overflow info
	or.l	%d1,%d0
# return (exc);				/* it is already in %d0 */
# }
	unlk	%fp
	rts
