#	@(#)getegid.s	6.1 
# C library -- getegid

# gid = getegid();
# returns effective gid

	file	"getegid.s"
	set	getgid%,47
	global	getegid

getegid:
	MCOUNT
	mov.l	&getgid%,%d0
	trap	&0
	mov.l	%d1,%d0
	rts
