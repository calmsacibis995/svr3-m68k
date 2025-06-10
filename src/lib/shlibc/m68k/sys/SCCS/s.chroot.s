h25441
s 00019/00000/00000
d D 1.1 86/07/31 12:29:58 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
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
E 1
