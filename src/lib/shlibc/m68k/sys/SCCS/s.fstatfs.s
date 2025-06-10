h49863
s 00030/00000/00000
d D 1.1 86/07/31 12:32:42 fnf 1 0
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

#ident	"@(#)libc-m32:sys/fstatfs.s	1.1"
# C library -- fstatfs

# int
# fstatfs(fd, sp, len, fstyp);
# int fd;
# struct statfs *sp;
# int len, fstyp;

	file	"fstatfs.s"
	set	fstatfs%,82
	global	fstatfs
	global  cerror%

fstatfs:
	MCOUNT
	mov.l	&fstatfs%,%d0
	trap	&0
	bcc.b 	noerror
	jmp 	cerror%
noerror:
	mov.l	&0,%d0
	rts
E 1
