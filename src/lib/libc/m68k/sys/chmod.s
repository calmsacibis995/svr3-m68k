#	@(#)chmod.s	6.1 
# C library -- chmod

# error = chmod(string, mode);

	file	"chmod.s"
	set	chmod%,15
	global	chmod
	global	cerror%

chmod:
	MCOUNT
	mov.l	&chmod%,%d0
	trap	&0
	bcc.b 	noerror
	jmp 	cerror%
noerror:
	mov.l	&0,%d0
	rts
