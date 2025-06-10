#
#	120io.s - module for I/O using MVME12x trap #15 calls
#			and subroutine calls
#
#
#		Copyright (c) 1986 Motorola Inc.
#		All Rights Reserved
#		%W%
#

	set	RSAV,0x3f3e
	set	RRES,0x7cfc
	set	ARG,48
	set  	BASE,0xf00000		# BUG address
	set	OUTCHR,BASE+0x34	# outchar
	set	GETSER,BASE+0x44	# get serial port
#
#	putchar - print one character
#
#	putchar(c)
#
	global putchar
putchar:
	movm.l	&RSAV,-(%sp)		# save registers
putc:	bsr.l	&GETSER			# get serial port
	mov.l	ARG(%sp),%d0		# receive the argument
	bsr.l	&OUTCHR			# goto monitor branch table
	cmp.l	ARG(%sp),&0x0a		# if it is LF,
	beq.b	putcr			# send a CR
	movm.l	(%sp)+,&RRES		# restore registers
	rts				# return
putcr:	mov.l	&0x0d,ARG(%sp)		# store a CR in the argument area
	bra.b	putc			# and send it

#
#	gobug - return to 120BUG
#
#	gobug()
#
	global	gobug
gobug:	
	movm.l	iregs,&0x0100		# restore a0
	trap	&15			# shouldn't return
	short	0x0
	bra.b	gobug			# but just in case

#
#	rdblk - read a block from the MVME350
#
#	rdblk(&paramblk)
#
#	paramater block offsets
#
	set A0,0x20
	set BLKS,0x0			# number of blocks
	set ADDR,0x4			# address for read
	set FILENO,0x8			# file number
	set STAT,0xc			# status
	set PKT,0x10			# 350packet address
#
#	READT table address
#
	set READT,BASE+0x000048

	global	rdblk
rdblk:
	movm.l	&RSAV,-(%sp)		# save registers
	mov.l	ARG(%sp),%a3		# parameter block address
	mov.l	BLKS(%a3),%d0		# number of blocks to read
	mov.l	ADDR(%a3),%d2		# load address
	mov.l	FILENO(%a3),%d3		# file number
	lea.l	iregs,%a1		# restore a0
	mov.l	A0(%a1),%a0		# 
	bsr.l	&READT			# goto read routine
	mov.l	%d0,STAT(%a3)		# move status from read
	mov.l	%a1,PKT(%a3)		# address of 350packet
	clr.l	%d0			# return 0
	tst.l	%d1			# 350 error
	bne.b	rderr			# go process read error
rdret:	movm.l	(%sp)+,&RRES		# restore registers
	rts				# return
rderr:	mov.l	&-1,%d0			# read error
	bra.b	rdret			# go return
