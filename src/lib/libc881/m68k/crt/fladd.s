#	@(#)fladd.s	6.1	

	file	"fladd.s"

#+
#
#	M68000 IEEE Format Single Precision Routines
#
#	(C) Copyright 1985 by Motorola Inc.
#
#
#  "fladd%%" and "flsub%%" are callable from 68k C.
#  They support ieee single precision floating point add and subtract.
#
#
#-

        text
 
	global	fladd%%,flsub%%
 
flsub%%:	
#
# float flsub(a,b)
# float a,b;
	set	a_off,8		# offsets of a and b from %fp
	set	b_off,12
# {
	bchg	&7,b_off-4(%sp)	# negate second operand
fladd%%:	
#
# float fladd(a,b)
# float a,b;
# {
	link.l	%fp,&-4
#	register float tmp_flt = a;
	fmov.s	a_off(%fp),%fp0
# tmp_flt += b;
	fadd.s	b_off(%fp),%fp0

# return (tmp_flt);
	fmov.s	%fp0,%d0
# }
	unlk	%fp
	rts
