h26964
s 00019/00000/00000
d D 1.1 86/07/31 12:30:10 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
#	@(#)mknod.s	6.1 
# C library -- mknod

# error = mknod(string, mode, major.minor);

	file	"mknod.s"
	set	mknod%,14
	global	mknod
	global  cerror%

mknod:
	MCOUNT
	mov.l	&mknod%,%d0
	trap	&0
	bcc.b 	noerror
	jmp 	cerror%
noerror:
	mov.l	&0,%d0
	rts
E 1
