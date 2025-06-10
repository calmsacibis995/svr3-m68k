#	@(#)fltodb.s	6.1	
#	@(#)fltodb.s	6.1	
#
#	M68000 IEEE Format Double Precision Routines
#
#	(C) Copyright 1985 by Motorola Inc.
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
	text	0
#
# double fltodb(a)
# float a;
# {
	global fltodb%%	
fltodb%%: 
	link.l	%fp,&-12
# double tmpdbl;
# register double tmpresult = a;
	fmov.s	%d0,%fp0
# tmpdbl = (double) tmpresult;
	fmov.d	%fp0,-8(%fp)

# return (tmpdbl);
	mov.l	-8(%fp),%d0
	mov.l	-4(%fp),%d1
# }
L%thru:
	unlk	%fp
	rts
