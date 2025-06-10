h37236
s 00036/00000/00000
d D 1.1 86/07/31 12:00:30 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
#	@(#)udiv.s	6.1 
#
#	udiv - unsigned division for vax-11
#
#	arguments: dividend, divisor.
#	result: quotient.
#	uses r0-r2
#
#	If 1 < divisor <= 2147483647, zero-extend the dividend
#	to 64 bits and let ediv do the work.  If the divisor is 1,
#	ediv will overflow if bit 31 of the dividend is on, so
#	just return the dividend unchanged.  If the divisor is 0,
#	do the ediv also, so it will generate the proper exception.
#	All other values of the divisor have bit 31 on: in this case
#	the quotient must be 0 if divisor > dividend, and 1 otherwise,
#	provided that the comparison is made as unsigned.
#
	.text
	.align	2
	.globl	udiv
udiv:	.word	0x0000
	movl	4(ap),r0	# Dividend
	movl	8(ap),r2	# Divisor
	jeql	div		# If divisor=0, force exception
	cmpl	r2,$1		# If divisor <= 1 (signed),
	jleq	nodiv		#  no division is necessary
div:	clrl	r1		# Zero-extend the dividend
	ediv	r2,r0,r0,r2	# Divide.  q->r0, r->r2 (discarded)
	ret
nodiv:	jeql	retn		# If divisor=1, return dividend
	cmpl	r0,r2		# Unsigned comparison between
	jgequ	one		#  dividend and divisor
	clrl	r0		# Dividend < divisor, return 0
	ret
one:	movl	$1,r0		# Dividend >= divisor, return 1
retn:	ret
E 1
