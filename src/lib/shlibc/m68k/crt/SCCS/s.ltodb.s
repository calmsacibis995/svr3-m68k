h09626
s 00113/00000/00000
d D 1.1 86/07/31 12:00:19 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
# "@(#)ltodb.s	6.1	";
	file	"ltodb.s"

#+
#
#	M68000 IEEE Format Double Precision Routines
#
#	(C) Copyright 1983 by Motorola Inc.
#
#	Written by: Jim Hanko and Eric Randau
#
#
#
#  no error conditions possible in these routines
#
# globals: 
#        untoflt --- unsigned 32 bit integer to ieee d.p.
#        sntoflt ---   signed 32 bit integer to ieee d.p.
#
# xrefs: none
#-
 
#	register mask,etc
	set	reglist,0x00f0	# %d4-%d7
	set	nreg,-4*4
 
        text
	global	ultodb%%	
	global	ltodb%%	
 
 
ltodb%%:	link	%fp,&-2
	clr.w	(%sp)	# assume int. is +
	tst.l	%d0	# check sign
	bpl.b	L%plus	# was +
	mov.w	&-1,(%sp)	# flag as -
	neg.l	%d0	# 2's compl. integer
	bvs.b	L%maxneg	# overflow, was max. negative integer
L%plus:	bsr.b	ultodb%%	# convert to ieee double precision
	tst.w	(%sp)	# check sign flag
	beq.b	L%ret	# was +
	bset	&31,%d0	# set ieee sign bit
	bra.b	L%ret	# return
 
L%maxneg:	mov.l	&0xc1e00000,%d0	# set result to max. neg. int. (in d.p.)
	mov.l	&0,%d1	
L%ret:	unlk	%fp	
	rts
 
 
 
 
 
ultodb%%:	
	link	%fp,&nreg
	movm.l	&reglist,(%sp)	# protect registers %d4-%d7
	mov.l	%d0,%d1	# put integer in %d1
	beq.b	L%return	# if zero return it (%d0,%d1 = 0)
	mov.l	&0,%d0	# prepare to convert to d.p.
	mov.l	&52,%d6	# initialize exponent
#	while.l	%d0	 <le> &0x1f do.s
	bra.b	L%endw1
L%while1:
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
L%endw1:
	cmp.l	%d0,&0x1f
	ble.b	L%while1
#	if.l	%d0	# <le> &0x1fff then.s
	cmp.l	%d0,&0x1fff
	bgt.b	L%endi1
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
L%endi1:
#	btst	&20,%d0	
#	while	<eq>	 do.s
	bra.b	L%endw2
L%while2:
# shift the mantissa zero or more times (up to seven).
	add.l	%d1,%d1	# shift left by adding
	addx.l	%d0,%d0	# shift extend bit in
	sub.w	&1,%d6	
#	endw
L%endw2:
	btst	&20,%d0	
	beq.b	L%while2
	add.w	&0x3ff,%d6	# add offset to actual exponent
	lsl.l	&4,%d6	# left justify exponent
	swap	%d6	
	bclr	&20,%d0	# remov "implied bit"
	or.l	%d6,%d0	# insert exponent
L%return:	movm.l	(%sp),&reglist	# pop nonvolatile regs
	unlk	%fp
        rts
 
#	end
E 1
