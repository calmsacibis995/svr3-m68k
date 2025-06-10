#	@(#)itof.s	6.1	

	file	"itof.s"

#+
#
#	M68000 IEEE Format Single Precision Routines
#
#	(C) Copyright 1985 by Motorola Inc.
#
#
#	"itof%%" and "ltof" are callable from 68k C.
#	They allow the user to convert integers and longs
#	to single precision floating point.
#
#
#-
 
        text
 
	global	itof%%,ltof%%
 
itof%%:	
ltof%%:
#
# fp itof(a)
# int a;
	set	a_off,8		# offset of a from %fp
# {
	link.l	%fp,&-4
# register float tmp_flt = a;
	fmov.l	a_off(%fp),%fp0

# return (tmp_flt);
	fmov.s	%fp0,%d0
# }
	unlk	%fp
	rts
