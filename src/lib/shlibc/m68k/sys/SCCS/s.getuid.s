h21480
s 00015/00000/00000
d D 1.1 86/07/31 12:30:06 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
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
E 1
