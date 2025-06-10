#	@(#)getgid.s	6.1 
# C library -- getgid

# gid = getgid();
# returns real gid

	file	"getgid.s"
	set	getgid%,47
	global	getgid

getgid:
	MCOUNT
	mov.l	&getgid%,%d0
	trap	&0
	rts
