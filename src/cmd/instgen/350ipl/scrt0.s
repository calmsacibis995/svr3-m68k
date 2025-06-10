#
#	scrt0.s: Standalone C RunTime startup
#
#
#		Copyright (c) 1986 Motorola Inc.
#		All Rights Reserved
#		%W%
#
	data
	global	iregs
iregs:	long	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0	# initial registers

#
#
#

	text
stack:
	long	stack
	long	start

#
#	for debugging purposes
#
	long	0,0, 0,0,0,0, 0,0,0,0
#
#
#	start IPL
#
	global	start
	global	_start
_start:
start:
	mov.l	&stack,%a7
	movm.l	&0xffff,iregs
#
#	zero uninitialized data
#
	mov.l	&0,%d0
	mov.l	&edata,%a0
	mov.l	&end,%d1
clrlp:	mov.l	&0,(%a0)+
	cmp.l	%a0,%d1
	ble.b	clrlp
#
#	clear all registers
#
	mov.l	&0,%d1
	mov.l	&0,%d2
	mov.l	&0,%d3
	mov.l	&0,%d4
	mov.l	&0,%d5
	mov.l	&0,%d6
	mov.l	&0,%d7
	mov.l	&0,%a0
	mov.l	&0,%a1
	mov.l	&0,%a2
	mov.l	&0,%a3
	mov.l	&0,%a4
	mov.l	&0,%a5
	mov.l	&0,%a6
#
#	jump to main
#
	mov.l	&0,-(%sp)	# null argv
	mov.l	&0,-(%sp)	# zero argc
	jsr	main		# go run program
	jsr	gobug		# go back to bug

#
#	jumpos: jump to operating system after restoring registers
#
#	jumpos(start address)
#
	global	jumpos
jumpos:	add.l	&4,%sp		# pop off unnecessary return address
	movm.l	iregs,&0x7fff	# restore all but stack pointer
#	clr.l	%d0		# device zero
#	clr.l	%d1		# controller zero
#	clr.l	%d2		# inode number
	rts			# jump to operating system
