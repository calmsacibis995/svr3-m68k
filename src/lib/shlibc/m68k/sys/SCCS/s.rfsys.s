h39055
s 00022/00000/00000
d D 1.1 86/07/31 12:32:50 fnf 1 0
c Pick up version Matt did for 5.3 m68k library.
e
u
U
t
T
I 1
#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)libc-m32:sys/rfsys.s	1.3"

	file	"rfsys.s"
	set	rfsys%,78
	global	rfsys	
	global	cerror%

rfsys:
	MCOUNT
	mov.l	&rfsys%,%d0
	trap	&0
	bcc.b 	noerror
	jmp 	cerror%
noerror:
	rts
E 1
