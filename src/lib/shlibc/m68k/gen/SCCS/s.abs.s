h25498
s 00018/00000/00000
d D 1.1 86/07/31 12:16:50 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
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
E 1
