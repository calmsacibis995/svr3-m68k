# "@(#)dbadd.s	6.1	";
	file	"dbadd.s"

#+
#
#	M68000 IEEE Format Double Precision Routines
#
#	(C) Copyright 1983 by Motorola Inc.
#
#	Written by: Jim Hanko and Eric Randau
#
#
#  dbadd and dbsub are callable from 68k C
#  they support ieee double precision floating point
#  unnormalized numbers cause an underflow... underflow gives
#  no warning.
#  nans and
#  overflow cause return +infinity
#
#
# dbadd entry point for double prec. add, operands in %d0-%d1, 4(%sp)
# dbsub entry point for double prec. sub, operands in %d0-%d1, 4(%sp)
#
#-
 
# register masks

	set	nreg,-7*4
	set	reglist,0x04fc	#%a2,%d2-%d7

 
        text
 
	global	dbadd%%; global dbsub%%	
 

	text
 
dbsub%%:	
#        on  exit,  <%d0,%d1> = <%d0,%d1> - <4(%sp),8(%sp)>
	bchg	&7,4(%sp)	# negate second operand
dbadd%%:	
#
#        on exit,   <%d0,%d1> = <%d0,%d1> + <4(%sp),8(%sp)>
#
	link	%fp,&nreg
	movm.l	&reglist,(%sp)
	mov.l	8(%fp),%d2	# pull off second operand
	mov.l	12(%fp),%d3
	mov.l	%d2,%d5	# get exponent/sign
	swap	%d5	
	lsr.w	&4,%d5	# exponent to %d5
	and.l	&0xfffff,%d2	# remov exponent from mantissa
	bset	&20,%d2	# real mantissa
	mov.l	%d0,%d4	# get exponent/sign
	swap	%d4	
	lsr.w	&4,%d4	# exponent to %d4
	and.l	&0xfffff,%d0	# remov exponent from mantissa
	bset	&20,%d0	# real mantissa
	mov.w	%d4,%d6	
	mov.w	%d5,%d7	
	and.w	&0x7ff,%d6	# remov sign
	beq.b	L%chksec	# unnormalized, check second operand
	cmp.w	%d6,&0x7ff	# check for nan or infinity
	beq.b	L%ovrflo	# error- nan or infinity
# here iff first & is n-o-t nan, inf or unnorm.
	and.w	&0x7ff,%d7	# remov sign
	beq	L%rtrnit	# unnorm or zero, return first operand
	cmp.w	%d7,&0x7ff	# nan or inf?
	beq.b	L%ovrflo	# error-
	sub.w	%d7,%d6	
	beq.b	L%samemg	# extend is zero from sub
	bpl.b	L%flarge	
# force first arg to have larger exponent
	exg	%d0,%d2	
	exg	%d1,%d3	
	exg	%d4,%d5	
	neg.w	%d6	
L%flarge:	cmp.w	%d6,&53	# if >53 different, second is effectively
	bgt	L%rtrnit	# zero, so just return first
	cmp.w	%d6,&32	
	blt.b	L%short	
# fast shift right by 32 or more bits ... 'X' bit has highest 'lost' bit
	sub.w	&32,%d6	
	add.l	%d3,%d3	# force msb into extend bit by shifting left
	mov.l	%d2,%d3	
	mov.l	&0,%d2	
# now shift the rest if >32
	lsr.l	%d6,%d3	# extend is msb of lost info
	bra.b	L%samemg	
 
L%chksec:	and.w	&0x7ff,%d7	# first is unnorm or zero
	beq	L%iszero	# and 2nd is unnorm or zero
	cmp.w	%d7,&0x7ff	# is 2nd nan or inf?
	beq.b	L%ovrflo	# error-
	exg	%d0,%d2	# 2nd is a valid number, return it
	exg	%d1,%d3	# swaps 2nd and first
	exg	%d4,%d5	
	bra	L%rtrnit	
 
L%ovrflo:
	mov.l	&0x7ff00000,%d0	# return + infinity
	mov.l	&0,%d1
	bra	L%return
 
# shift by <32 bits
L%short:	mov.w	%d6,%d7	
	asl.w	&2,%d6	
	ror.l	%d7,%d2	
	lsr.l	%d7,%d3	# extend is msb of lost info
	lea	table%%,%a0	# %a0 --> table
	mov.l	0(%a0,%d6.w),%d7	
	and.l	%d2,%d7	
	or.l	%d7,%d3	# put in lsw
	eor.l	%d7,%d2	# remov from msw
L%samemg:	mov.w	%d5,%d7	
	eor.w	%d4,%d7	
	and.w	&0x800,%d7	# check signs
	beq	L%doadd	# same signs..no problem - do an add
# shift in the 'lost' bit for the subtract
	addx.l	%d3,%d3	# "shift %d3 left by 1 with x"
	addx.l	%d2,%d2	# "shift %d2 left by 1 with x"
	add.l	%d1,%d1	# "shift %d1 left by 1"
	addx.l	%d0,%d0	# "shift %d0 left by 1 with x"
	sub.l	%d3,%d1	
	subx.l	%d2,%d0	
	bpl.b	L%subnrm	
# second number really larger, negate result
	neg.l	%d1	# double precision negate
	negx.l	%d0	
	bchg	&11,%d4	# change sign of result
L%subnrm:	tst.l	%d0	
	bne.b	L%nonzro	
	tst.l	%d1	
	beq.b	L%iszero	
L%nonzro:	mov.w	%d4,%d6	# put exponent in %d6
	and.w	&0x7ff,%d6	# strip sign bit
	btst	&21,%d0	# see if we must shift back due to using 'lost' bit
#	if	<ne>	 then.s
	beq.b	L%else1
	lsr.l	&1,%d0	# shift upper half of mantissa
	roxr.l	&1,%d1	# shift lower half of mantissa
#	else		# else we lost precision ... must shift back
	bra	L%endi1
L%else1:
	sub.w	&1,%d6	# adjust the exponent (due to earlier shift)
#	while.l	%d0	# <le> &0x1f do.s  (shift by 16 bits)
L%while1:
	cmp.l	%d0,&0x1f 
	bgt.b	L%endw1
# this "while" will shift the mantissa 16 bits at a time. this can
# occur zero or more times (up to a maximum of 3).
# the algorithm is:
# each lower case letter represents 16 bits.
# the initial state is ----> %d0 =ab ,  %d1= cd
	swap	%d0	# %d0=ba
	swap	%d1	# %d1=dc
	mov.w	%d1,%d0	# %d0=bc , %d1=dc
	clr.w	%d1	# %d0=bc , %d1=d0
	sub.w	&16,%d6	# adjust the exponent
#	endw
	bra.b	L%while1
L%endw1:
#	if.l	%d0	# <le> &0x1fff then.s
	cmp.l	%d0,&0x1fff
	bgt.b	L%endi2
# this "if" will shift the mantissa 8 bits at a time.
# the algorithm is:
# each lower case letter represents 8 bits.
# the initial state is ----> %d0=abcd , %d1=efgh
	lsl.l	&8,%d0	# %d0=bcd0 , %d1=efgh
	rol.l	&8,%d1	# %d0=bcd0 , %d1=fghe
	mov.b	%d1,%d0	# %d0=bcde , %d1=fghe
	clr.b	%d1	# %d0=bcde , %d1=fgh0
	sub.w	&8,%d6	# adjust the exponent
#	endi
L%endi2:
#	btst	&20,%d0	
#	while	<eq>	do.s
	bra.b	L%endw2
# shift the mantissa zero or more times (up to seven).
L%while2:
	add.l	%d1,%d1	# shift left by adding
	addx.l	%d0,%d0	# shift extend bit in
	sub.w	&1,%d6	
L%endw2:
	btst	&20,%d0	
	beq.b   L%while2
#	endw
#	endi		# end denormalized
L%endi1:
	tst.w	%d6	# if exponent is <= zero (underflow)
	ble.b	L%iszero	# then set the result to zero
	and.w	&0x800,%d4	
	or.w	%d6,%d4	# get new exponemt with old sign
	bra.b	L%rtrnit	
#
L%iszero:	mov.l	&0,%d0	# make exponent and mantissa zero!
	mov.l	&0,%d1	
L%return:
	movm.l	(%sp),&reglist	# pop nonvolatile regs
	unlk	%fp
         rts
 
#
# at this point (doadd) the extend bit will be a zero --- or in the case
# that one of the operands had to be unnormalized, the extend bit will
# be the last bit shifted out during the "unnormalization"
#
 
L%doadd:	roxl.b	&1,%d6	# save the extend bit in %d6 to round result
	add.l	%d3,%d1	# do the actual add
	addx.l	%d2,%d0	
	mov.l	&0,%d3	# clear %d3 for "addx.l #0" later
	roxr.b	&1,%d6	# restore the old "extend bit"
	btst	&21,%d0	# unnormalized?
	beq.b	L%round	# no
	add.w	&1,%d4	# add to exponent (check for exponent ov later)
	lsr.l	&1,%d0	# normalize by shifting right one
	roxr.l	&1,%d1	
L%round:	addx.l	%d3,%d1	# round up
	addx.l	%d3,%d0	
	btst	&21,%d0	# unnormalized?
	beq.b	L%rtrnit	# no
	lsr.l	&1,%d0	# normalize by shifting right one place
	roxr.l	&1,%d1	
	add.w	&1,%d4	# increment exponent
L%rtrnit:	mov.w	%d4,%d6	# ensure exponent did not overflow
	and.w	&0x7ff,%d6	
	cmp.w	%d6,&0x7ff	
	beq	L%ovrflo	# error-
	lsl.w	&4,%d4	# shift exponent with 0 in l. s. nibble
	swap	%d4	# put in upper word
	clr.w	%d4	# and clear lower word
	bclr	&20,%d0	# clear 'hidden bit'
	or.l	%d4,%d0	# insert sign/exponent in %d0 (answer)
	movm.l	(%sp),&reglist	# pop nonvolatile regs
	unlk	%fp
         rts
 
#	end
 
