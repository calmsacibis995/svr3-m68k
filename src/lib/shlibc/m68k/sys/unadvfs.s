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
