h23064
s 00016/00000/00000
d D 1.1 86/07/31 12:30:22 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
#	@(#)ulimit.s	6.1 
# C library -- ulimit

	file	"ulimit.s"
	set	ulimit%,63
	global	ulimit
	global	cerror%

ulimit:
	MCOUNT
	mov.l	&ulimit%,%d0
	trap	&0
	bcc.b	noerror
	jmp	cerror%
noerror:
	rts
E 1
