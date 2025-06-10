#	@(#)ioctl.s	6.1 
# C library -- ioctl

	file	"ioctl.s"
	set	ioctl%,54
	global	ioctl
	global	cerror%

ioctl:
	MCOUNT
	mov.l	&ioctl%,%d0
	trap	&0
	bcc.b	noerror
	jmp	cerror%
noerror:
	rts
