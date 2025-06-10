#	@(#)abs.s	6.1 
#	/* Assembler program to implement the following C program */
#	int
#	abs(arg)
#	int	arg;
#	{
#		return((arg < 0)? -arg: arg);
#	}

	file	"abs.s"
	global	abs
abs:
	MCOUNT
	mov.l	4(%sp),%d0
	bge.b	absl
	neg.l	%d0
absl:
	rts
