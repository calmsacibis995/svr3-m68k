h21635
s 00016/00000/00000
d D 1.1 86/07/31 12:30:07 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
#	@(#)gtty.s	6.1 
# C library -- gtty

	file	"gtty.s"
	set	gtty%,32
	global	gtty
	global	cerror%

gtty:
	MCOUNT
	mov.l	&gtty%,%d0
	trap	&0
	bcc.b	noerror
	jmp	cerror%
noerror:
	rts
E 1
