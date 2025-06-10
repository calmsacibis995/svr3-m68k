h16675
s 00053/00000/00000
d D 1.1 86/07/31 12:16:58 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
#	@(#)memchr.s	6.1	
#
#	M68000 Memory(3C) Routine
#
#	(C) Copyright 1983 by Motorola Inc.
#
#	Written by: Steve Sheahan
#
# memchr - Returns a pointer to the first occurrence of character c in
#	   the first n characters of memory area s1, or NULL if c does
#	   not occur
	file	"memchr.s"
#
# Input:	s1 - memory area in which to conduct search
#		 c - character being sought
#		 n - maximum number of characters to search
#
# Output:	pointer to character 'c' or NULL
#
					# char *
					# memchr(s1, c, n)
					# register char *s1, c;
					# register int n;
					# {
	text
	global	memchr
memchr:
	mov.l	4(%sp),%a0		# addr(s1)
	mov.b	11(%sp),%d0		# c
	mov.l	12(%sp),%d1		# n
					# while (--n >= 0)
	ble.b	L%3
	bra.b	L%2
L%0:
	swap	%d1			# lower half = 2**16 - 1 = 0xFFFF
					# if (*s1++ == c)
L%1:
	cmp.b	%d0,(%a0)+
L%2:
	dbeq	%d1,L%1 
	beq.b	L%4			# did loop terminate because of match
	swap	%d1
	dbra	%d1,L%0			# decrement upper half of %d1
L%3:
					# return (0);
	sub.l	%a0,%a0
	bra.b	L%5
L%4:
					# return (--s1);
	sub.l	&1,%a0
L%5:
	mov.l	%a0,%d0
	rts
E 1
