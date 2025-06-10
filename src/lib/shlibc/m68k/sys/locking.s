#		Copyright (c) 1985 AT&T		#
#		All Rights Reserved		#
#	@(#)locking.s	6.1 
# C library -- locking -- J. Bass version of /usr/group file/record locking
# Added to V/68 along with os/locking.c -- 3/3/86 -wjc

	file	"locking.s"
	set	locking%,56
	global	locking
	global	cerror%

locking:
	MCOUNT
	mov.l	&locking%,%d0
	trap	&0
	bcc.b	noerror
	jmp	cerror%
noerror:
	rts
