h28349
s 00019/00000/00000
d D 1.1 86/07/31 12:30:31 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
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
E 1
