#	@(#)chdir.s	6.1 
# C library -- chdir

# error = chdir(string);

	file	"chdir.s"
	set	chdir%,12
	global	chdir
	global	cerror%

chdir:
	MCOUNT
	mov.l	&chdir%,%d0
	trap	&0
	bcc.b 	noerror
	jmp 	cerror%
noerror:
	mov.l	&0,%d0
	rts
