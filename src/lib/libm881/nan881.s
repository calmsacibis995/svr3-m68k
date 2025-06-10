#	@(#)nan881.s	6.1	


	file	"nan881.s"

# int nan881(a,b)
# double a,b;
# {
	text
	global	nan881
nan881:
	link.l	%fp,&-4
	mov.l	&1,%d0		# assume NaN found to start
# a = b;
	ftest.d	(8,%fp)
	fbun	done		# exit because a is a NaN

	ftest.d	(16,%fp)
	fbun	done		# b is a NaN

	clr.l	%d0		# neither a nor b is a NaN
# }
done:
	unlk	%fp
	rts
