#	@(#)creat.s	6.1 
# C library -- creat

# file = creat(string, mode);
# file == -1 if error

	file	"creat.s"
	set	creat%,8
	global	creat
	global	cerror%

creat:
	MCOUNT
	mov.l	&creat%,%d0
	trap	&0
	bcc.b 	noerror
	jmp 	cerror%
noerror:
	rts
