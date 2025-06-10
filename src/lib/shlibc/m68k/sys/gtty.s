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
