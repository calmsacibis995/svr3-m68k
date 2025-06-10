#	@(#)umask.s	6.1 
#  C library -- umask
 
#  omask = umask(mode);
 
	file	"umask.s"
	set	umask%,60
	global	umask
	global	cerror%

umask:
	MCOUNT
	mov.l	&umask%,%d0
	trap	&0
	bcc.b	noerror
	jmp	cerror%
noerror:
	rts
