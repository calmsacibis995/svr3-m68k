#	@(#)setgid.s	6.1 
# C library -- setgid

# error = setgid(uid);

	file	"setgid.s"
	set	setgid%,46
	global	setgid
	global  cerror%

setgid:
	MCOUNT
	mov.l	&setgid%,%d0
	trap	&0
	bcc.b 	noerror
	jmp 	cerror%
noerror:
	mov.l	&0,%d0
	rts
