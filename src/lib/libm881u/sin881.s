#	@(#)sin881.s	6.1	


	file	"sin881.s"

# unsigned int
# sin881(x,y)
# double x;
# double *y;
# {
	set	F%1,-4
	global	sin881
sin881:
	link.l	%fp,&F%1
# unsigned register int exc;		/* imagine exc as being %d0 */
# exc = fsin(x,&y);
	fsin.d (8,%fp),%fp0
	fmovm.l	%status,%d0		# contains any exception or
					# condition code info
	fmov.d	%fp0,([16.w,%fp])
# return (exc);				/* it is already in %d0 */
# }
	unlk	%fp
	rts


# unsigned int
# cos881(x,y)
# double x;
# double *y;
# {
	set	F%2,-4
	global	cos881
cos881:
	link.l	%fp,&F%2
# unsigned register int exc;		/* imagine exc as being %d0 */
# exc = fcos(x,&y);
	fcos.d (8,%fp),%fp0
	fmovm.l	%status,%d0		# contains any exception or
					# condition code info
	fmov.d	%fp0,([16.w,%fp])
# return (exc);				/* it is already in %d0 */
# }
	unlk	%fp
	rts
