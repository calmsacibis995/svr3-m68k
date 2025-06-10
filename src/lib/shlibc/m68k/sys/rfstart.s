#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)libc-m32:sys/rfstart.s	1.2"

	file	"rfstart.s"
	set	rfstart%,74
	global	rfstart
	global	cerror%

rfstart:
	MCOUNT
	mov.l	&rfstart%,%d0
	trap	&0
	bcc.b 	noerror
	jmp 	cerror%
noerror:
	rts
