#	@(#)dbneg.s	6.1	
#
#	M68000 IEEE Format Double Precision Routines
#
#	(C) Copyright 1983 by Motorola Inc.
#
#	Written by: Steve Sheahan
#
# dbneg: change the sign of a double precision number
	file	"dbneg.s"

# Input:  %d0, %d1 - double precision input
#
# Output: %d0, %d1 - double precision output with sign changed
#

	text	0
	global	dbneg%%
dbneg%%:
	btst	&31,%d0
	beq.b	L%toneg
	bclr	&31,%d0		#   change to a positive number
	rts
L%toneg:
	bset	&31,%d0		#   change to a negative number
	rts
