# "@(#)dbmul.s	6.1	";
	file	"dbmul.s"

#+
#
#	M68000 IEEE Format Double Precision Routines
#
#	(C) Copyright 1983 by Motorola Inc.
#
#	Written by: Jim Hanko and Eric Randau
#
#
#  dbmul is callable from 68k C
#  it supports ieee double precision floating point.
#  unnormalized numbers cause an underflow... underflow gives
#  no warning.
#  nans and
#  overflow return + infinity
#
# dbmul entry point for double prec. mul, operands in %d0-%d1, 4(%sp) %d0-%d1=result.
#
# xrefs: none
#-
# offsets for stack save area	
# NOTE : source2 must follow source1 in memory for the movm to work

	set	result,-16	# ds.l	4	
	set	source2,result-8	#ds.l	2	
	set	source1,source2-8	# ds.l	2
	set	size,source1
 
 # register masks

	set	nreg,-7*4
	set	reglist,0x04fc	#%a2,%d2-%d7

         text
	global	dbmul%%	
 
dbmul%%:	
#
#        on exit,   <%d0,%d1> = <%d0,%d1> * <4(%sp),8(%sp)>
#
	link	%fp,&(nreg+size)
	movm.l	&reglist,(%sp)	# push nonvolatile regs
	mov.l	8(%fp),%d2
	mov.l	12(%fp),%d3
#
# a little trick here: we skip the inner loop if a word of operand 1 is
# zero, so we'll switch operands if the second half of operand 2 is zero 
#  - saves half the time spent in the inner loop!
#
	bne.b	L%noswap
	exg	%d0,%d2
	exg	%d1,%d3
L%noswap:
	mov.l	%d2,%d5	# get exponent/sign of op2
	swap	%d5	
	lsr.w	&4,%d5	# exponent to %d5
	and.l	&0xfffff,%d2	# remov exponent from mantissa
	bset	&20,%d2	# real mantissa
	mov.l	%d0,%d4	# get exponent/sign of op1
	swap	%d4	
	lsr.w	&4,%d4	# exponent to %d4
	and.l	&0xfffff,%d0	# remov exponent from mantissa
	bset	&20,%d0	# real mantissa
	mov.w	%d4,%d6	
	mov.w	%d5,%d7	
	and.w	&0x7ff,%d6	# remov sign
	beq	L%chksec	# unnormalized, check second operand
	cmp.w	%d6,&0x7ff	# check for nan or infinity
	beq	L%ovrflo	# error- nan or infinity
# here iff first & is n-o-t nan, inf or unnorm.
	and.w	&0x7ff,%d7	# remov sign
	beq	L%iszero	# unnorm or zero, return zero (0*n = 0)
	cmp.w	%d7,&0x7ff	# nan or inf?
	beq	L%ovrflo	# error-
	add.w	%d7,%d6	# add exponents
	sub.w	&0x3ff,%d6	# adjust for 'excess 1023' arithmetic
	ble	L%iszero	# result is too small - underflow
	cmp.w	%d6,&0x7ff	# see if result overflows
	bge	L%ovrflo	
	eor.w	%d5,%d4	# compute sign
	and.w	&0x800,%d4	
	or.w	%d6,%d4	# result's sign and exponent in %d4
#
	mov.l	&0,%d5	# put 16 bytes of zeros on stack for result
	mov.l	%d5,result(%fp)	
	mov.l	%d5,result+4(%fp)	
	mov.l	%d5,result+8(%fp)	
	mov.l	%d5,result+12(%fp)	
	movm.l	&0xf,source1(%fp)	# push operands %d0-%d3
#
#        multi-precision unsigned multiply using memory arrays
#        a la knuth "the art of computer programming" volume 2
#
	mov.l	&0,%d3	# a zero word
	lea	source1+6+2(%fp),%a0	# 1 word past end of source1
	mov.l	&6,%d5	# loop count*2 (used to index words)
#	repeat
L%rep1:
	mov.w	-(%a0),%d7	# get next operand for inner loop
	beq.b	L%skip	# skip the loop if zero (saves 4 multiplies, etc.)
	lea	source2+6+2(%fp),%a1	# 1 word past end of source2
	lea	result+6+4(%fp,%d5.w),%a2	# the right place for the result
	mov.l	&0,%d2	# used as 'carry' word
	mov.l	&3,%d6	# loop counter
L%mloop:	
	mov.w	-(%a1),%d0	
	mulu	%d7,%d0	# t:=a[i]*b[j]
	add.l	%d0,%d2	# t:=t+carry
	add.w	%d2,-(%a2)	# r[i+j]:=(t+r[i+j]) mod 64k
	clr.w	%d2	# carry:=(t+r[i+j]) div 64k
	swap	%d2	
	addx.w	%d3,%d2	# {include 'extend' from word add}
	dbra	%d6,L%mloop	
	mov.w	%d2,-(%a2)	# write out most significant word
L%skip:
	sub.w	&2,%d5	
#	until	<mi>	
	bpl.b	L%rep1
#
# now pick up the real result
#
	mov.l	result+2(%fp),%d0	
	mov.l	result+6(%fp),%d1	
#
# put the 'units' bit in the right place
#
	ror.l	&4,%d0	
	lsr.l	&4,%d1	
	mov.l	%d0,%d5	
	and.l	&0xf0000000,%d5	# get them
	or.l	%d5,%d1	# mov them
	eor.l	%d5,%d0	# and remov them
	mov.l	&0,%d5	
	btst	&21,%d0	# see if unnormalized
	beq.b	L%round	# normalized
	add.w	&1,%d4	# add to exponent (check for exponent ov later)
	lsr.l	&1,%d0	# normalize by shifting right one
	roxr.l	&1,%d1	
L%round:	addx.l	%d5,%d1	# add back extend bit to round according
	addx.l	%d5,%d0	# to last bit shifted out
	btst	&21,%d0	# see if unnormalized
	beq.b	L%rtrnit	# is ok
	lsr.l	&1,%d0	# normalize by shifting right one
	roxr.l	&1,%d1	
	add.w	&1,%d4	# add to exponent
L%rtrnit:	mov.w	%d4,%d6	# ensure exponent did not overflow
	and.w	&0x7ff,%d6	
	cmp.w	%d6,&0x7ff	
	bge.b	L%ovrflo	# error-
	lsl.w	&4,%d4	# shift exponent with 0 in l. s. nibble
	swap	%d4	# put in upper word
	clr.w	%d4	# and clear lower word
	bclr	&20,%d0	# clear 'hidden bit'
	or.l	%d4,%d0	# insert sign/exponent in %d0 (answer)
L%return:	
	movm.l	(%sp),&reglist	# pop nonvolatile regs
	unlk	%fp
         rts
 
L%chksec:	and.w	&0x7ff,%d7	# first is unnorm or zero
	cmp.w	%d7,&0x7ff	# is 2nd nan or inf?
	beq.b	L%ovrflo	# error-
	bra.b	L%iszero	# else return zero (0*n = 0)
 
L%ovrflo:	mov.l	&0x7ff00000,%d0	# return + infinity
	mov.l	&0,%d1
	bra.b	L%return
 
L%iszero:	mov.l	&0,%d0	# make exponent and mantissa zero!
	mov.l	&0,%d1	
	movm.l	(%sp),&reglist	# pop nonvolatile regs
	unlk	%fp
         rts
 
#	end
 
