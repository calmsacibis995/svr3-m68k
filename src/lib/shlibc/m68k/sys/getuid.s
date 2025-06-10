#	@(#)getuid.s	6.1 
# C library -- getuid

# uid = getuid();
#  returns real uid

	file	"getuid.s"
	set	getuid%,24
	global	getuid

getuid:
	MCOUNT
	mov.l	&getuid%,%d0
	trap	&0
	rts
