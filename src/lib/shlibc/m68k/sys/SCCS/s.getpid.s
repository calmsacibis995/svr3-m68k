h18855
s 00012/00000/00000
d D 1.1 86/07/31 12:30:05 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
#	@(#)getpid.s	6.1 
# getpid -- get process ID

	file	"getpid.s"
	set	getpid%,20
	global	getpid

getpid:
	MCOUNT
	mov.l	&getpid%,%d0
	trap	&0
	rts
E 1
