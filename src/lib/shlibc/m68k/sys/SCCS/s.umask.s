h24244
s 00018/00000/00000
d D 1.1 86/07/31 12:30:23 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
#	@(#)umask.s	6.1 
#  C library -- umask
 
#  omask = umask(mode);
 
	file	"umask.s"
	set	umask%,60
	global	umask
	global	cerror%

umask:
	MCOUNT
	mov.l	&umask%,%d0
	trap	&0
	bcc.b	noerror
	jmp	cerror%
noerror:
	rts
E 1
