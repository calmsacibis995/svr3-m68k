#	@(#)dbcmp.s	6.1	
#	@(#)dbcmp.s	6.1	
#
#	M68000 IEEE Format Double Precision Routines
#
#	(C) Copyright 1985 by Motorola Inc.
#
# dbcmp: compares two double precision numbers: "x" and "y"
	file	"dbcmp.s"

# Input: %d0, %d1 first double precision number ("x")
#	 4(%sp)   most significant long word of 2nd argument  ("y")
#	 8(%sp)   least significant long word of 2nd argument
#
# Output: %d0 =  0 if x == y or x or y (or both) is a NaN
#	      =  1 if x > y
#	      = -1 if x < y
#
# Function:
#	This function compares two double precision numbers and returns the
#	the result of this comparison in %d0
#

	text	0
	global	dbcmp%%
dbcmp%%:
#
# dbcmp(a,b)
# double a,b;
# {
	link.l	%fp,&-12
# double tmpdbl;
# register double tmpresult;
# tmpdbl = a;
	mov.l	%d0,-8(%fp)
	mov.l	%d1,-4(%fp)
# tmp_a = tmpdbl;
	fmov.d	-8(%fp),%fp0
	clr.l	%d0			# set equal or NaN as default return
# if (tmp_a == b || tmp_a .un. b)
	fcmp.d	%fp0,8(%fp)
	fbueq.w	L%thru			# a and b are equal or incomparable
	fbolt.w L%neg			# a < b
	mov.l	&1,%d0			# a > b
	bra.b	L%thru

L%neg:	mov.l	&-1,%d0

# }
L%thru:
	unlk	%fp
	rts
