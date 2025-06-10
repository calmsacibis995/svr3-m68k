h17111
s 00012/00000/00000
d D 1.1 86/07/31 12:30:20 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
#	@(#)sync.s	6.1 
#  C library -- sync

	file	"sync.s"
	set	sync%,36
	global	sync

sync:
	MCOUNT
	mov.l	&sync%,%d0
	trap	&0
	rts
E 1
