#	@(#)lseek.s	6.1 
# C library -- lseek

# error = lseek(file, offset, ptr);

	file	"lseek.s"
	set	lseek%,19
	global	lseek
	global  cerror%

lseek:
	MCOUNT
	mov.l	&lseek%,%d0
	trap	&0
	bcc.b 	noerror
	jmp 	cerror%
noerror:
	rts
