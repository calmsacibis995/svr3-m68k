#	@(#)uitof.s	6.1	

	file	"uitof.s"

#+
#
#	M68000 IEEE Format Single Precision Routines
#
#	(C) Copyright 1985 by Motorola Inc.
#
#
#	"uitof%%" and "ultof" are callable from 68k C.
#	They allow the user to convert unsigned integers and longs
#	to single precision floating point.
#
#
#-
 
        text
 
	global	uitof%%,ultof%%
 
uitof%%:	
ultof%%:
#
# fp uitof(a)
# register unsigned int a;
	set	a_off,8		# offset of a from %fp
# {
	link.l	%fp,&-4
# register float tmp_res;
# register long big_num = 0;
	clr.l	%d1
# put "a" in a register
	mov.l	a_off(%fp),%d0
# if (a > max_unsigned)			# bigger than any unsigned #
	bpl.b	L%small
#	{
# a -= 0x80000000;
	and.l	&0x7fffffff,%d0		# make it fit unsigned pos.  range
# big_num = 1;
	mov.l	&1,%d1
#	}
L%small:
# tmp_res = a;
	fmov.l	%d0,%fp0
# if (big_num)
	tst.l	%d1
	beq.b	L%notbig
#	{
#		tmp_res +=  0x80000000;
	mov.l	&0x40000000,%d1
	fadd.l	%d1,%fp0
	fadd.l	%d1,%fp0		# add 0x80000000 to our float res
#	}
L%notbig:
# return( tmp_res);
	fmov.s	%fp0,%d0
# }
	unlk	%fp
	rts
