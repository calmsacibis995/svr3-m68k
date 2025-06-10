h52882
s 00037/00000/00000
d D 1.1 86/07/31 12:17:04 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
#	@(#)strcpy.s	6.1	
#
#	M68000 String(3C) Routine
#
#	(C) Copyright 1983 by Motorola Inc.
#
#	Written by: Steve Sheahan
#
# strcpy - copies string s2 to s1, stopping after the null character has 
#	   been copied
	file	"strcpy.s"
#
# Input:	pointer to strings s1 and s2
#
# Output:	pointer to s1
#
# Registers:	%a0 - points at s1
#		%a1 - points at s2

					# char *
					# strcpy(s1, s2)
					# register char *s1, *s2;
					# {
	text
	global	strcpy
strcpy:
	mov.l	4(%sp),%a0		# addr(s2)
	mov.l	8(%sp),%a1		# addr(s1)

					# while(*s1++ = *s2++)
L%1:
	mov.b	(%a1)+,(%a0)+
	bne.b	L%1
					# return(s1);
	mov.l	4(%sp),%a0
	mov.l	%a0,%d0
	rts
E 1
