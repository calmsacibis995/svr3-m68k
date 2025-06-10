#	@(#)getpid.s	6.1 
# getpid -- get process ID

	file	"getpid.s"
	set	getpid%,20
	global	getpid

getpid:
	MCOUNT
	mov.l	&getpid%,%d0
	trap	&0
	rts
