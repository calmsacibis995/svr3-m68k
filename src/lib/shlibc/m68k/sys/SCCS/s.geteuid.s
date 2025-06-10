h23545
s 00016/00000/00000
d D 1.1 86/07/31 12:30:04 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
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
E 1
