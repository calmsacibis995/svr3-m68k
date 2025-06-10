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
