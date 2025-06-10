#	@(#)dbdiv.s	6.1	
# "@(#)dbdiv.s	6.1	";
	file	"dbdiv.s"

#+
#
#	M68000 IEEE Format Double Precision Routines
#
#	(C) Copyright 1985 by Motorola Inc.
#
#
#  dbdiv is callable from 68k C
#  it supports ieee double precision floating point.
#  unnormalized numbers cause an underflow... underflow gives
#  no warning. this includes those underflows that cause a divide by
#  zero.... divide by zero returns + infinity
#  overflow returns +infinity, as does a NaN input.
#
#-
	set	EXC_OPERR,0x2000
	set	EXC_DZ,0x0400
	set	FPCC_NAN,0x01000000

        text
	global	dbdiv%%	
 
dbdiv%%:	
#
#        on exit,   <%d0,%d1> = <%d0,%d1> / <4(%sp),8<%sp>
#                  quotient = dividend / divisor
#
# double dbdiv(a,b)
# double a,b;
# {
	link.l	%fp,&-12
# double tmpdbl;
# register double tmpresult;
# tmpdbl = a;
	mov.l	%d0,-8(%fp)
	mov.l	%d1,-4(%fp)
# tmpresult = tmpdbl;
	fmov.d	-8(%fp),%fp0
# tmpresult /= b;
	fdiv.d	8(%fp),%fp0

	fmovm.l	%status,%d0
	and.l	&EXC_OPERR+FPCC_NAN+EXC_DZ,%d0
	beq.b	L%okay
	mov.l	&0x7ff00000,%d0		# NaNs/illegal divides set to +infinity
	clr.l	%d1
	bra.b	L%thru

L%okay:
# tmpdbl = tmpresult;
	fmov.d	%fp0,-8(%fp)
# return (tmpdbl);
	mov.l	-8(%fp),%d0
	mov.l	-4(%fp),%d1
# }
L%thru:
	unlk	%fp
	rts
