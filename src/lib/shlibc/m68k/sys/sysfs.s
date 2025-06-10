#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)libc-m32:sys/sysfs.s	1.2"
# C library -- sysfs

	file	"sysfs.s"
	set	sysfs%,84
	global	sysfs
	global	cerror%

sysfs:
	MCOUNT
	mov.l	&sysfs%,%d0
	trap	&0
	bcc.b	noerror
	jmp	cerror%
noerror:
	rts
