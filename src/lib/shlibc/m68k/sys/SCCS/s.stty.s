h21713
s 00016/00000/00000
d D 1.1 86/07/31 12:30:20 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
#	@(#)stty.s	6.1 
# C library -- stty

	file	"stty.s"
	set	stty%,31
	global	stty
	global	cerror%

stty:
	MCOUNT
	mov.l	&stty%,%d0
	trap	&0
	bcc.b	noerror
	jmp	cerror%
noerror:
	rts
E 1
