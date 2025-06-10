h18009
s 00122/00000/00000
d D 1.1 86/07/31 12:00:05 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
# "@(#)dbtol.s	6.1	";
	file	"dbtol.s"

#+
#
#	M68000 IEEE Format Double Precision Routines
#
#	(C) Copyright 1983 by Motorola Inc.
#
#	Written by: Jim Hanko and Eric Randau
#
#
#        entry : <%d0,%d1> = ieee d.p. number
#        exit  : <%d0>    = signed (or unsigned) 32 bit integer
#        fltolng --- ieee d.p. to 32 bit signed integer
#        fltouln --- ieee d.p. to 32 bit unsigned integer
#
# xrefs: none
#-
 
# set register masks
	set	nreg,-4*4
	set	reglist,0x00f0
 

	text
	global dbtol%%,dbtoul%%
 
dbtol%%:	
#        on entry,  <%d0,%d1> = ieee d.p. number
#        on  exit,  <%d0>    = a truncated (not rounded) 32 bit signed integer.
#
#        an overflow results in the largest integer of propper sign
#
	link	%fp,&-2
	tst.l	%d0	# save a copy of the sign.
	bpl.b	L%plus	# sign is plus
	mov.w	&0xffff,(%sp)	# flag as minus
	cmp.l	%d0,&0xc1e00000	# check for max. neg. integer in fp representation
	bne.b	L%over	# can't be -2147483648
	tst.l	%d1	# check ls word for max. neg. integer
	bne.b	L%over	# continue
L%bigneg:	# return largest neg number
	mov.l	&0x80000000,%d0	# return -2147483648
	unlk	%fp
        rts
 
L%plus:	clr.w	(%sp)	# flag as plus
L%over:	bclr	&31,%d0	# make number positive
	bsr.b	dbtoul%%	# convert to unsigned long integer (32 bits)
	tst.l	%d0	# if ms bit is set, we have an overflow condition
	bmi	L%ovrflo	# error
	tst.w	(%sp)	# pull "sign flag"
	beq.b	L%retlng	# converted value is +
	neg.l	%d0	# converted value is - (neg is 2's complement)
L%retlng:	# return with 32 bit result in %d0
	unlk	%fp
        rts
 
L%ovrflo:	# too big for signed number
	tst.w	(%sp)
	bmi.b	L%bigneg
	mov.l	&0x7fffffff,%d0	# return largest positive integer
	unlk	%fp
        rts

dbtoul%%:	
#        on entry,  <%d0,%d1> = ieee d.p. number
#        on exit,   <%d0>    = a truncated (not rounded) 32bit unsigned integer
#
#        an overflow results in the largest unsigned integer
#
	link	%fp,&nreg
	movm.l	&reglist,(%sp)	# push nonvolatile regs
	tst.l	%d0	# error if number is negative, return 0
	bmi	L%ret0
	mov.l	%d0,%d5	# get exponent/sign
	swap	%d5	
	lsr.w	&4,%d5	# right justify exponent
	and.l	&0xfffff,%d0	# remov exponent from mantissa
	bset	&20,%d0	# insert "hidden" bit
	and.w	&0x7ff,%d5	# remov sign
	cmp.w	%d5,&0x7ff	# check for nan or infinity
	beq.b	L%ovflow	# error-
	sub.w	&0x3ff,%d5	# remov offset from exponent (excess 1023)
	bmi.b	L%ret0	# if exponent is <0, return 0
	cmp.w	%d5,&32	# if exponent is > 32 it will oveflow a long integer
	bge.b	L%ovflow	# error
	neg.w	%d5	# 2's complement exponent
	add.w	&52,%d5	# result= number of shifts rsetired to right justify
	cmp.w	%d5,&32	# decide method to justify fastest.
	blt.b	L%short	# fastest for 31 or less shifts
	mov.l	%d0,%d1	# shift by 32 places
	sub.w	&32,%d5	# adjust exponent to reflect 32 bit shift
	lsr.l	%d5,%d1	# shift the remaining number of places
	bra.b	L%justif	
 
L%short:	mov.w	%d5,%d7	
	asl.w	&2,%d5	# mul "# of shifts" by 4 to get proper offset into table
	ror.l	%d7,%d0	
	lsr.l	%d7,%d1	
	lea	table%%,%a0	# %a0 --> table
	mov.l	0(%a0,%d5.w),%d7	
	and.l	%d0,%d7	
	or.l	%d7,%d1	# put in lsw
L%justif:	# here the integer part is right justified
	mov.l	%d1,%d0	# put resulting integer in %d0
	movm.l	(%sp),&reglist	# restore saved registers
	unlk	%fp
        rts
#
L%ret0:	mov.l	&0,%d0	# resulting integer =0
	movm.l	(%sp),&reglist	# restore saved registers
	unlk	%fp
        rts
# 
L%ovflow:	mov.l	&-1,%d0	# return largest unsigned (all 1's)
	movm.l	(%sp),&reglist	# restore saved registers
	unlk	%fp
        rts
 
#	end
E 1
