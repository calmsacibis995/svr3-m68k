#	@(#)chown.s	6.1 
# C library -- chown

# error = chown(string, owner);

	file	"chown.s"
	set	chown%,16
	global	chown
	global	cerror%

chown:
	MCOUNT
	mov.l	&chown%,%d0
	trap	&0
	bcc.b 	noerror
	jmp 	cerror%
noerror:
	mov.l	&0,%d0
	rts
