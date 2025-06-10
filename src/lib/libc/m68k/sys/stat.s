#	@(#)stat.s	6.1 
# C library -- stat

# error = stat(string, statbuf);
# char statbuf[36]

	file	"stat.s"
	set	stat%,18
	global	stat
	global  cerror%

stat:
	MCOUNT
	mov.l	&stat%,%d0
	trap	&0
	bcc.b 	noerror
	jmp 	cerror%
noerror:
	mov.l	&0,%d0
	rts
