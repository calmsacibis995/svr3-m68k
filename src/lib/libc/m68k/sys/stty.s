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
