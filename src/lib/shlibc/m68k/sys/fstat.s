#	@(#)fstat.s	6.1 
# C library -- fstat

# error = fstat(file, statbuf);

# char statbuf[34]

	file	"fstat.s"
	set	fstat%,28
	global	fstat
	global	cerror%

fstat:
	MCOUNT
	mov.l	&fstat%,%d0
	trap	&0
	bcc.b 	noerror
	jmp 	cerror%
noerror:
	mov.l	&0,%d0
	rts
