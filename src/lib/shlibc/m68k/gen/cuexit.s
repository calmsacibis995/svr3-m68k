#	@(#)cuexit.s	6.1 
# C library -- exit

# exit(code)
# code is return in r0 to system

	file	"cuexit.s"
	set	exit%,1
	global	exit
	global	_cleanup

exit:
	MCOUNT
	jsr	_cleanup
	mov.l	&exit%,%d0
	trap	&0
	stop	&0
