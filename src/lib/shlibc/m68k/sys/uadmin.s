#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)libc-m32:sys/uadmin.s	1.3"
# C library -- uadmin

# file = uadmin(cmd,fcn[,mdep])

# file == -1 means error

	file	"uadmin.s"
	set	uadmin%,55
	global	uadmin
	global  cerror%

uadmin:
	MCOUNT
	mov.l	&uadmin%,%d0
	trap	&0
	bcc.b 	noerror
	jmp 	cerror%
noerror:
	rts
