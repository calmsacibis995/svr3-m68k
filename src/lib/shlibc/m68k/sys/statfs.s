#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)libc-m32:sys/statfs.s	1.1"
# C library -- statfs

# int
# statfs(filename, sp, len, fstyp);
# char *filename;
# struct statfs *sp;
# int len, fstyp;

	file	"statfs.s"
	set	statfs%,35
	global	statfs
	global  cerror%

statfs:
	MCOUNT
	mov.l	&statfs%,%d0
	trap	&0
	bcc.b 	noerror
	jmp 	cerror%
noerror:
	mov.l	&0,%d0
	rts
