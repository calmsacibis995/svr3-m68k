h26929
s 00020/00000/00000
d D 1.1 86/07/31 12:30:18 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
#	@(#)stat.s	6.1 
# C library -- stat

# error = stat(string, statbuf);
# char statbuf[36]

	file	"stat.s"
	set	stat%,18
	global	stat
	global  cerror%

stat:
	MCOUNT
	mov.l	&stat%,%d0
	trap	&0
	bcc.b 	noerror
	jmp 	cerror%
noerror:
	mov.l	&0,%d0
	rts
E 1
