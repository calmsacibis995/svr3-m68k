h20082
s 00091/00000/00000
d D 1.1 86/07/31 12:00:12 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
#	@(#)fltodb.s	6.1	
#
#	M68000 IEEE Format Double Precision Routines
#
#	(C) Copyright 1983 by Motorola Inc.
#
#	Written by: Steve Sheahan
#
# fltodb: float to double conversion
	file	"fltodb.s"

# Input:  %d0 - contains the single precision number
#
# Output: %d0, %d1 - contains the double precision number
#
# Function:
#	This routine converts a single precision number into a double precision
#	number. This conversion abides by the IEEE standard. Unnormalized
#	numbers are normalized. NaNs are converted to double precision NaNs.
#	Single precision infinity is converted to double precision infinity.
#	The sign of the double precision number is always that of the single
#	precision number.
#
# Register usage:
#	%a0, %a1: are used for temporary storage of %d2 and %d3
#	%d0	: the sign bit is always maintained in this register along
#		  with the beginning bits of the fractionO
#	%d1	: holds the least significant 32 bits of the result
#	%d2	: work space for exponent computations
#	%d3	: temporary storage area

	text	0
	global fltodb%%	

fltodb%%: 
	mov.l	%d2,%a0
	mov.l	%d3,%a1
				# isolate the exponent in d2 and
				# L%shift it to double precision location
	mov.l	%d0,%d2		
	clr.w 	%d2		 
	swap.w	%d2
	and.w   &0x7f80,%d2	 
	lsr.w	&3,%d2
				# if exponent == 0 &&
	bne.b	L%chkinf
				#    fraction != 0
	tst.l	%d0
	beq.b	L%shift
				   # then normalize number
	mov.l	%d0,%d3		   # copy to preserve sign bit
	mov.w	&0x3810,%d2	   # exponent = 897 (1023 - 126)
L%norm:
	asl.l	&1,%d0		   # searching for most significant bit
	sub.w	&0x10,%d2	   # exp = exp - 1
	btst	&23,%d0		   # is this bit set?
	beq.b	L%norm
	and.l	&0x80000000,%d3	   # d3 = sign bit
	and.l   &0x007fffff,%d0    # save fraction
	or.l	%d3,%d0            # restore sign bit
	bra.b	L%shift		   # number is now normalized

				# else if exponent == INFINITY
L%chkinf: 
	cmp.w	%d2,&0x0ff0        # remember the exponent has been shifted
	bne.b	L%repres
	mov.w	&0x7ff0,%d2	   # set exponent to double precision infinity
	bra.b	L%shift

				# else exponent is in the normal range
L%repres: 
	add.w	&0x3800,%d2	   # add 896 (1023 - 127) to exponent

				# make room for the new exponent in d0
				# by L%shifting the last three bits of d0
				# to the three most significant bits of d1
L%shift:
	mov.l	%d0,%d1
	and.l	&0x00000007,%d1	# save the last three bits
	mov.l	&29,%d3		# prepare to shift %d1
	lsl.l	%d3,%d1		# %d1 is now complete

	asr.l	&3,%d0		# %d0 now contains the sign and the first
				# 20 bits of the fraction
	and.l	&0x800fffff,%d0	# zero out the exponent
	swap.w	%d2		# restore %d2
	or.l	%d2,%d0		# or the exponent in its proper position

	mov.l	%a0,%d2		# restore registers
	mov.l	%a1,%d3
	rts
E 1
