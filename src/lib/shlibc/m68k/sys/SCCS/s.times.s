h22264
s 00016/00000/00000
d D 1.1 86/07/31 12:30:22 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
#	@(#)times.s	6.1 
# C library -- times

	file	"times.s"
	set	times%,43
	global	times
	global	cerror%

times:
	MCOUNT
	mov.l	&times%,%d0
	trap	&0
	bcc.b	noerror
	jmp	cerror%
noerror:
	rts
E 1
