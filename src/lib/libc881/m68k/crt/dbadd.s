#	@(#)dbadd.s	6.1	
# "@(#)dbadd.s	6.1	";
	file	"dbadd.s"

#+
#
#	M68000 IEEE Format Double Precision Routines
#
#	(C) Copyright 1985 by Motorola Inc.
#
#
#  dbadd and dbsub are callable from 68k C
#  they support ieee double precision floating point
#  unnormalized numbers cause an underflow... underflow gives
#  no warning.  nans and overflow cause return +infinity.
#
#
# dbadd entry point for double prec. add, operands in %d0-%d1, 4(%sp)
# dbsub entry point for double prec. sub, operands in %d0-%d1, 4(%sp)
#
#-
 
	set	FPCC_NAN,0x01000000

        text
 
	global	dbadd%%; global dbsub%%	
 
dbsub%%:	
#        on  exit,  <%d0,%d1> = <%d0,%d1> - <4(%sp),8(%sp)>
	bchg	&7,4(%sp)	# negate second operand
dbadd%%:	
#
#        on exit,   <%d0,%d1> = <%d0,%d1> + <4(%sp),8(%sp)>
#
# dbadd(a,b)
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
# tmpresult += b;
	fadd.d	8(%fp),%fp0
# tmpdbl = tmpresult;
	fmov.d	%fp0,-8(%fp)

	fmovm.l	%status,%d0
	and.l	&FPCC_NAN,%d0
	beq.b	L%okay
	mov.l	&0x7ff00000,%d0		# NaNs set to +infinity, as before
	clr.l	%d1
	bra.b	L%thru

# return (tmpdbl);
L%okay:
	mov.l	-8(%fp),%d0
	mov.l	-4(%fp),%d1
# }
L%thru:
	unlk	%fp
	rts
