#	@(#)unlink.s	6.1 
# C library -- unlink

# error = unlink(string);

	file	"unlink.s"
	set	unlink%,10
	global	unlink
	global  cerror%

unlink:
	MCOUNT
	mov.l	&unlink%,%d0
	trap	&0
	bcc.b 	noerror
	jmp 	cerror%
noerror:
	mov.l	&0,%d0
	rts
