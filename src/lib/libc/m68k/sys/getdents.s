#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# C library -- getdents
#	ident	"@(#)libc-m32:sys/getdents.s	1.2.1.1"
	file 	"getdents.s"

# num = getdents(file, buffer, count, flag);
# num is number of bytes read; num == -1 means error

	set	getdents%,81
	global	getdents
	global  cerror%

getdents:
	MCOUNT
	mov.l	&getdents%,%d0
	trap	&0
	bcc.b 	noerror
	jmp 	cerror%
noerror:
	rts
