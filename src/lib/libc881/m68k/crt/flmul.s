#	@(#)flmul.s	6.1	

	file	"flmul.s"

#+
#
#	M68000 IEEE Format Single Precision Routines
#
#	(C) Copyright 1985 by Motorola Inc.
#
#
#  "flmul%%" is callable from 68k C.
#  It supports ieee single precision floating point multiply.
#
#
#-
 
        text
 
	global	flmul%%
 
flmul%%:	
#
# float flmul(a,b)
# float a,b;
	set	a_off,8		# offsets of a and b from %fp
	set	b_off,12
# {
	link.l	%fp,&-4
# register float tmp_flt = a;
	fmov.s	a_off(%fp),%fp0
# tmp_flt *= b;
	fmul.s	b_off(%fp),%fp0

# return (tmp_flt);
	fmov.s	%fp0,%d0
# }
	unlk	%fp
	rts
