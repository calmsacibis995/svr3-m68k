#	@(#)setuid.s	6.1 
# C library -- setuid

# error = setuid(uid);

	file	"setuid.s"
	set	setuid%,23
	global	setuid
	global  cerror%

setuid:
	MCOUNT
	mov.l	&setuid%,%d0
	trap	&0
	bcc.b 	noerror
	jmp 	cerror%
noerror:
	mov.l	&0,%d0
	rts
