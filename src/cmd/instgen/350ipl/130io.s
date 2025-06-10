#
#	130io.s - module for I/O using 130BUG trap #15 calls
#
#
#		Copyright (c) 1986 Motorola Inc.
#		All Rights Reserved
#		%W%
#

	set	RSAV,0x3f3e
	set	RRES,0x7cfc
	set	ARG,48
#
#	putchar - print one character
#
#	putchar(c)
#
	global putchar
putchar:
	movm.l	&RSAV,-(%sp)		# save registers
putc:	mov.l	ARG(%sp),%d0		# receive the argument
	mov.b	%d0,-(%sp)		# push
	trap	&15
	short	0x0020			# .outchr
	cmp.l	ARG(%sp),&0x0a		# if it is LF,
	beq.b	putcr			# send a CR
	movm.l	(%sp)+,&RRES		# restore registers
	rts				# return
putcr:	mov.l	&0x0d,ARG(%sp)		# store a CR in the argument area
	bra.b	putc			# and send it

#
#	gobug - return to 130BUG
#
#	gobug()
#
	global	gobug
gobug:	
	trap	&15			# shouldn't return
	short	0x0063
	bra.b	gobug			# but just in case

#
#	rdblk - read a block from the MVME350
#
#	rdblk(&paramblk)
#

	set BLKCNT,0xc			# blkcnt in packet
	global	rdblk
rdblk:
	movm.l	&RSAV,-(%sp)		# save registers
	mov.l	ARG(%sp),%a2		# address of packet
	mov.l	%a2,-(%sp)		# push parameter block address
	trap	&15
	short	0x0010			# read block
	bne.b	rderr			# go process read error

	mov.l	&0,%d0			# clear LONG d0 before getting the WORD
					#blkcnt !, since 'C' uses a long for
					# error returns
	mov.w	BLKCNT(%a2),%d0		# number of blocks xferd
rdret:
	movm.l	(%sp)+,&RRES		# restore registers
	rts				# return
rderr:	mov.l	&-1,%d0			# read error
	bra.b	rdret			# go return
