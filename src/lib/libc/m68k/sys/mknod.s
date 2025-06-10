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
