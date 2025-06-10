h25290
s 00018/00000/00000
d D 1.1 86/07/31 12:30:09 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
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
E 1
