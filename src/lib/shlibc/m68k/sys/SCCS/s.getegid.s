h23377
s 00016/00000/00000
d D 1.1 86/07/31 12:30:03 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
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
E 1
