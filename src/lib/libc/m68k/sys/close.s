#	@(#)close.s	6.1 
# C library -- close

# error =  close(file);

	file	"close.s"
	set	close%,6
	global	close
	global	cerror%

close:
	MCOUNT
	mov.l	&close%,%d0
	trap	&0
	bcc.b 	noerror
	jmp 	cerror%
noerror:
	mov.l	&0,%d0
	rts
