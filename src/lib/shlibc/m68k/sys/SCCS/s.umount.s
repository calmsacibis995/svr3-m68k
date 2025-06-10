h24100
s 00017/00000/00000
d D 1.1 86/07/31 12:30:23 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
#	@(#)umount.s	6.1 
# C library -- umount

	file	"umount.s"
	set	umount%,22
	global	umount
	global	cerror%

umount:
	MCOUNT
	mov.l	&umount%,%d0
	trap	&0
	bcc.b 	noerror
	jmp 	cerror%
noerror:
	mov.l	&0,%d0
	rts
E 1
