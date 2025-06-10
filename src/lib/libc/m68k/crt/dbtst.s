#	@(#)dbtst.s	6.1	
#
#	M68000 IEEE Format Double Precision Routines
#
#	(C) Copyright 1983 by Motorola Inc.
#
#	Written by: Steve Sheahan
#
# dbtst: double precision test for zero and the sign of a variable
	file	"dbtst.s"

# Input:  %d0, %d1 - contains the double precision number
#
# Output: %d0 - 0 if the double precision number was equal to +/- 0
#		-1 if the double precision number was negative
#		1 if the double precision number was positive
#
# Exceptions:	NANs are treated as zero
#
	text	0
	global	dbtst%%
dbtst%%:	
	mov.l	%d0,%d1		# make a copy of the exponent
	and.l	&0x7ff00000,%d1	# if exponent = 0 then result = 0  (NANs = 0)
	beq.b	L%return
	mov.l	&1,%d1		# assume number is positive
	btst	&31,%d0		# test sign bit
	beq.b	L%return
	mov.l	&-1,%d1		# number is negative
L%return:
	mov.l	%d1,%d0
	rts
