# "@(#)table.s	6.1	";
	file	"table.s"

#
#
#	M68000 IEEE Format Double Precision Routines
#
#	(C) Copyright 1983 by Motorola Inc.
#
#	Written by: Jim Hanko and Eric Randau
#
#
	global table%%

	text
table%%:	long	0	
	long	0x80000000	# "table" is used to do a "fast" shifting of
	long	0xc0000000	# 64 bits by 1 to 32 places. (for normalization)
	long	0xe0000000	
	long	0xf0000000	# holds the
	long	0xf8000000	# actual
	long	0xfc000000	# number (say 
	long	0xfe000000	# k) bits to mask
	long	0xff000000	# out
	long	0xff800000	
	long	0xffc00000	
	long	0xffe00000	
	long	0xfff00000	
	long	0xfff80000	
	long	0xfffc0000	
	long	0xfffe0000	
	long	0xffff0000	
	long	0xffff8000	
	long	0xffffc000	
	long	0xffffe000	
	long	0xfffff000	
	long	0xfffff800	
	long	0xfffffc00	
	long	0xfffffe00	
	long	0xffffff00	
	long	0xffffff80	
	long	0xffffffc0	
	long	0xffffffe0	
	long	0xfffffff0	
	long	0xfffffff8	
	long	0xfffffffc	
	long	0xfffffffe	
	long	0xffffffff	
