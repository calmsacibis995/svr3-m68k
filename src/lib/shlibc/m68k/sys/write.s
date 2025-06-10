#	@(#)write.s	6.1 
# C library -- write

# nwritten = write(file, buffer, count);
# nwritten == -1 means error

	file	"write.s"
	set	write%,4
	global	write
	global  cerror%

write:
	MCOUNT
	mov.l	&write%,%d0
	trap	&0
	bcc.b 	noerror
	jmp 	cerror%
noerror:
	rts
