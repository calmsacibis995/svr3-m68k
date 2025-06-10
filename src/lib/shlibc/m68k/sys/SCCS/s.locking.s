h36260
s 00019/00000/00000
d D 1.1 86/07/31 12:30:09 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
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
E 1
