#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)libc-m32:sys/putmsg.s	1.1"

# C library -- putmsg

	file	"putmsg.s"
	set	putmsg%,86
	global	putmsg
	global	cerror%

putmsg:
	MCOUNT
	mov.l	&putmsg%,%d0
	trap	&0
	bcc.b	noerror
	jmp	cerror%
noerror:
	rts
