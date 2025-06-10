#	@(#)read.s	6.1 
# C library -- read

# nread = read(file, buffer, count);
# nread ==0 means eof; nread == -1 means error

	file	"read.s"
	set	read%,3
	global	read
	global  cerror%

read:
	MCOUNT
	mov.l	&read%,%d0
	trap	&0
	bcc.b 	noerror
	jmp 	cerror%
noerror:
	rts
