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
