#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)libc-m32:sys/rdebug.s	1.3"

	file	"rdebug.s"
	set	rdebug%,76
	global	rdebug
	global	cerror%

rdebug:
	MCOUNT
	mov.l	&rdebug%,%d0
	trap	&0
	bcc.b 	noerror
	jmp 	cerror%
noerror:
	rts
