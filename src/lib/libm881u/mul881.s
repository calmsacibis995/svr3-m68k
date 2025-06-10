#	@(#)mul881.s	6.1	


	file	"mul881.s"

# int mul881(a,b)
# double a;
# double *b;
# {
	text
	global	mul881
mul881:
	link.l	%fp,&-4
# *b = *b * a;
	fmov.d	([16.w,%fp]),%fp0
	fmul.d	(8,%fp),%fp0
	fmovm.l	%status,%d0		# save status of the multiply
					# as the return value
	fmov.d	%fp0,([16.w,%fp])
	fmovm.l	%status,%d1		# may contain overflow info
	or.l	%d1,%d0
# }
	unlk	%fp
	rts
