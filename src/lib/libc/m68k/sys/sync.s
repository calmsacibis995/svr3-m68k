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
