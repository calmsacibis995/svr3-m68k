#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)libc-m32:sys/getmsg.s	1.1"

# C library -- getmsg

	file	"getmsg.s"
	set	getmsg%,85
	global	getmsg
	global	cerror%

getmsg:
	MCOUNT
	mov.l	&getmsg%,%d0
	trap	&0
	bcc.b	noerror
	jmp	cerror%
noerror:
	rts
