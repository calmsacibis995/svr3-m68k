#	@(#)chroot.s	6.1 
#  C library -- chroot
 
#  error = chroot(string);
 
	file	"chroot.s"
	set	chroot%,61
 
	global	chroot
	global	cerror%

chroot:
	MCOUNT
	mov.l	&chroot%,%d0
	trap	&0
	bcc.b	noerror
	jmp	cerror%
noerror:
	rts
