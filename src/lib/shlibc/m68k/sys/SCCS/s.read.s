h28193
s 00019/00000/00000
d D 1.1 86/07/31 12:30:15 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
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
E 1
