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
