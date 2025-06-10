#	@(#)geteuid.s	6.1 
# C library -- geteuid

# uid = geteuid();
#  returns effective uid

	file	"geteuid.s"
	set	getuid%,24
	global	geteuid

geteuid:
	MCOUNT
	mov.l	&getuid%,%d0
	trap	&0
	mov.l	%d1,%d0
	rts
