#	@(#)ftol.s	6.2	

	file	"ftol.s"

#+
#
#	M68000 IEEE Format Single Precision Routines
#
#	(C) Copyright 1985 by Motorola Inc.
#
#
#  "ftol%%" is callable from 68k C.
#  It supports ieee single precision floating point divide.
#
#
#-
 
        text
 
	global	ftol%%
 
ftol%%:	
#
# long ftol(a)
# fp a;
	set	a_off,8		# offset of a from %fp
# {
	link.l	%fp,&-4
# register float tmp_flt = a;
	fmov.s	a_off(%fp),%fp0
# truncate (tmp_flt);
	fintrz.x %fp0,%fp0
# return (tmp_flt);
	fmov.l	%fp0,%d0
# }
	unlk	%fp
	rts
