h40538
s 00023/00000/00000
d D 1.1 86/07/31 12:32:51 fnf 1 0
c Pick up version Matt did for 5.3 m68k library.
e
u
U
t
T
I 1
#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# C library -- rmdir
#	ident	"@(#)libc-m32:sys/rmdir.s	1.4"

	file	"rmdir.s"
	set	rmdir%,79
	global	rmdir
	global	cerror%

rmdir:
	MCOUNT
	mov.l	&rmdir%,%d0
	trap	&0
	bcc.b 	noerror
	jmp 	cerror%
noerror:
	rts
E 1
