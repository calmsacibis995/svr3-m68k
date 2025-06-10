#	@(#)fcntl.s	6.1 
# C library -- fcntl

	file	"fcntl.s"
	set	fcntl%,62
	global	fcntl
	global	cerror%

fcntl:
	MCOUNT
	mov.l	&fcntl%,%d0
	trap	&0
	bcc.b	noerror
	jmp	cerror%
noerror:
	rts
