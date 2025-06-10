#	@(#)mount.s	6.1 
# C library -- mount

# error = mount(dev, file, flag)

	file	"mount.s"
	set	mount%,21
	global	mount
	global  cerror%

mount:
	MCOUNT
	mov.l	&mount%,%d0
	trap	&0
	bcc.b 	noerror
	jmp 	cerror%
noerror:
	mov.l	&0,%d0
	rts
