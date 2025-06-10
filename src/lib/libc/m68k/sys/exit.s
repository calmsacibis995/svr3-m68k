#	@(#)exit.s	6.1 
# C library -- exit

# exit(code)
# code is %deturn in r0 to system
# Same as plain exit, for user who want to define their own exit.

	file	"exit.s"
	set	exit%,1
	global	_exit

_exit:
	MCOUNT
	mov.l	&exit%,%d0
	trap	&0
	stop	&0
