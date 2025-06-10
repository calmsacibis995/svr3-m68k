h14467
s 00051/00000/00000
d D 1.1 86/07/31 12:17:07 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
#	@(#)strpbrk.s	6.1	
#
#	M68000 String(3C) Routine
#
#	(C) Copyright 1983 by Motorola Inc.
#
#	Written by: Steve Sheahan
#
# strpbrk - return ptr to first occurrence of any character from `brkset'
#	    found in the character string `string'; NULL if none exists.
	file	"strpbrk.s"
#
# Input:	string - source string
#		brkset - characters sought for
#
# Output:	pointer to character found or NULL if none found
#
# Registers:	%a0 - string
#		%a1 - brkset
#		%d0 - scratch
#
					# char *
					# strpbrk(string, brkset)
					# register char *string, *brkset;
					# {
	text
	global	strpbrk
strpbrk:
	mov.l	4(%sp),%a0		# addr(string)
					# do {
L%1:
					# for(p=brkset; *p != '\0' 
					#		&& *p != *string; ++p);
	mov.l	8(%sp),%a1		# addr(brkset)
L%2:
	tst.b	(%a1)
	beq.b	L%3
	mov.b	(%a1)+,%d0
	cmp.b	%d0,(%a0)
	beq.b	L%4			# character found - terminate
	bra.b	L%2
					# while(*string++);
L%3:
	add.l	&1,%a0
	tst.b	(%a0)
	bne.b	L%1
					# return((char *) 0);
	sub.l	%a0,%a0
L%4:
	mov.l	%a0,%d0
	rts
E 1
