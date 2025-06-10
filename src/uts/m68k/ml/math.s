#		Copyright (c) 1985 AT&T		#
#		All Rights Reserved		#
#	@(#)math.m4	6.1 	

# MC68020 version

	text

# unsigned min(a,b)
# unsigned a,b;
# {	return a <= b ? a : b; }

	global	min
min:
	mov.l	4(%sp),%d0
	cmp.l	%d0,8(%sp)
	bls.b	L%min
	mov.l	8(%sp),%d0
L%min:
	rts

# unsigned max(a,b)
# unsigned a,b;
# {	return a >= b ? a : b; }

	global	max
max:
	mov.l	4(%sp),%d0
	cmp.l	%d0,8(%sp)
	bhs.b	L%max
	mov.l	8(%sp),%d0
L%max:
	rts

# log2(value)
# returns log2 value of passed parameter.
# Note:  Assumes only 1 bit set in passed parameter.  Bad results otherwise.

	global	log2
log2:
	mov.l	4(%sp),%d0		# Get number to be logged.
	bfffo	%d0{&0:&32},%d1		# Find offset to first 1.
	mov.l	&0x1f,%d0		# (From the wrong end)
	sub.l	%d1,%d0
	rts
