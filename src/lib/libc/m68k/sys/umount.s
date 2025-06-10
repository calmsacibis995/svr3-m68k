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
