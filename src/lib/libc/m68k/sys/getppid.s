#	@(#)getppid.s	6.1 
# getppid -- get parent process ID

	file	"getppid.s"
	set	getpid%,20
	global	getppid

getppid:
	MCOUNT
	mov.l	&getpid%,%d0
	trap	&0
	mov.l	%d1,%d0
	rts
