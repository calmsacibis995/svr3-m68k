h44546
s 00026/00000/00000
d D 1.1 86/07/31 12:32:44 fnf 1 0
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

# C library -- mkdir
#	ident	"@(#)libc-m32:sys/mkdir.s	1.3"
	file	"mkdir.s"

# file = mkdir(string, mode);
# file == -1 if error

	set	mkdir%,80
	global	mkdir
	global	cerror%

mkdir:
	MCOUNT
	mov.l	&mkdir%,%d0
	trap	&0
	bcc.b 	noerror
	jmp 	cerror%
noerror:
	rts
E 1
