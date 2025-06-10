#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)libc-m32:sys/netunboot.s	1.2"

	file	"netunboot.s"
	set	netunboot%,71
	global	netunboot
	global	cerror%

netunboot:
	MCOUNT
	mov.l	&netunboot%,%d0
	trap	&0
	bcc.b 	noerror
	jmp 	cerror%
noerror:
	rts
