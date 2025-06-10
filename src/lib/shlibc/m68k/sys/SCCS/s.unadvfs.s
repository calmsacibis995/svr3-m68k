h27011
s 00018/00000/00000
d D 1.1 86/07/31 12:32:55 fnf 1 0
c Pick up version Matt did for 5.3 m68k library.
e
u
U
t
T
I 1
#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#ident	"@(#)libc-m32:sys/unadvfs.s	1.3"

	file	"unadvfs.s"
	set	unadvfs%,71
	global	unadvfs
	global	cerror%

unadvfs:
	MCOUNT
	mov.l	&unadvfs%,%d0
	trap	&0
	bcc.b 	noerror
	jmp 	cerror%
noerror:
	rts
E 1
