#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)libc-m32:sys/poll.s	1.1"

# C library -- poll

	file	"poll.s"
	set	poll%,87
	global	poll
	global	cerror%

poll:
	MCOUNT
	mov.l	&poll%,%d0
	trap	&0
	bcc.b	noerror
	jmp	cerror%
noerror:
	rts
