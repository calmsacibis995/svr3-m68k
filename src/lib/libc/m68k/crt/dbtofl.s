#	@(#)dbtofl.s	6.1	
#
#	M68000 IEEE Format Double Precision Routines
#
#	(C) Copyright 1983 by Motorola Inc.
#
#	Written by: Steve Sheahan
#
# dbtofl: double to float conversion  
	file	"dbtofl.s"

# Input : %d0, %d1 - contains the double precision number
#
# Output: %d0 - contains the single precision number
#
# Function:
#	This routine converts a double precision number into a single precision
#	number. NaNs are treated like L%infinity.
#
# Register usage:
#	%a0, %a1: are used for temporary storage of %d2 and %d3
#	%d0	: the beginning bits of the fraction
#	%d1	: holds the least significant 32 bits of the input
#	%d2	: work space for exponent computations and eventually the sign
#	%d3	: temporary storage area

	text	0
	global	dbtofl%%

dbtofl%%:
	mov.l	%d2,%a0		# save %d2, %d3
	mov.l	%d3,%a1
				# isolate the exponent in d2 
	mov.l	%d0,%d2		
	clr.w 	%d2		 
	swap.w	%d2
	and.w   &0x7ff0,%d2	 
	cmp.w	%d2,&0x3680	# if exponent < 872  (1023 - 126 - 23 = 872)
	bge.b	L%tst2
L%zero:
	clr.l	%d0		#    result = 0.0
	bra.b	L%return

L%tst2:	
	cmp.w	%d2,&0x7ff0	# else if exponent == INFINITY (or number = NAN)
	beq.b   L%inf
	cmp.w	%d2,&0x47e0	#    || exponent >  1150 (1023 + 127 = 1150)
	ble.b	L%round
L%inf:
	and.l	&0x80000000,%d0	#    preserve sign
	or.l	&0x7f800000,%d0	#    result = +/- INFINITY
	bra.b	L%return

L%round:
				# else ROUND the fraction

	mov.l	%d0,-(%sp)	#    save sign bit
	lsl.l	&4,%d0		#    make room for 4 additional bits
	mov.l	&28,%d3
	lsr.l	%d3,%d1		#    reposition last 4 bits of fraction
	or.l	%d1,%d0		#    fraction now complete
	and.l	&0x00ffffff,%d0	#    zero out sign and exponent
	bset	&24,%d0		#    set the normalized bit
	add.l	&1,%d0		#    round fraction
	lsr.l	&1,%d0		#    delete round bit
	btst	&24,%d0		#    did number overflow?
	beq.b	L%chkunnorm	
	lsr.l	&1,%d0		#    overflowed occurred so normalize the number
	add.w	&0x0010,%d2	#    increment the exponent
				
L%chkunnorm:
				#    check to see if the number needs to 
				#    be represented by an unnormalized #

				#    if exponent <= 896 
	cmp.w	%d2,&0x3800
	bpl.b	L%addexp
				#       unnormalize number by shifting
				#	it to right (897-exponent) # of bits
	mov.w	&0x3810,%d3     #       %d3 = 897
	sub.w	%d2,%d3		
	lsr.w	&4,%d3
	lsr.l	%d3,%d0		#	number is now unnormalized
	mov.w	&0x3800,%d2	#       exponent = 896		

L%addexp:
	sub.w	&0x3800,%d2	#    exp = exp - 896
	lsl.w	&3,%d2		#    shift exponent to single precision location
	swap.w	%d2
				#    fraction and exponent have been computed
				#    merge the sign, exponent, and fraction
				#    together

	mov.l	(%sp)+,%d3	#    retrieve sign
	and.l	&0x80000000,%d3	
	and.l	&0x007fffff,%d0	#    clean up fraction
	or.l	%d3,%d2		#    %d2 holds the sign and the exponent
	or.l	%d2,%d0		#    %d0 = single precision number
L%return:
	mov.l	%a0,%d2		# restore registers
	mov.l	%a1,%d3
	rts
