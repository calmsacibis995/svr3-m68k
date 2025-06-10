#	@(#)dbtofl.s	6.1	
#	@(#)dbtofl.s	6.1	
#
#	M68000 IEEE Format Double Precision Routines
#
#	(C) Copyright 1985 by Motorola Inc.
#
# dbtofl: double to float conversion  
	file	"dbtofl.s"

# Input : %d0, %d1 - contains the double precision number
#
# Output: %d0 - contains the single precision number
#
# Function:
#	This routine converts a double precision number into a single precision
#	number. NaNs are treated like +infinity.
#
	text	0
	global	dbtofl%%
# float dbtofl(a)
# double a;
# {
dbtofl%%:
	link	%fp,&-12
# double tmpdbl;
# register double tmpresult;
# tmpdbl = a;
	mov.l	%d0,-8(%fp)
	mov.l	%d1,-4(%fp)
# tmpresult = tmpdbl;
	fmov.d	-8(%fp),%fp0
# return ( (float) tmpresult);
	fmov.s	%fp0,%d0
# }
	unlk	%fp
	rts
