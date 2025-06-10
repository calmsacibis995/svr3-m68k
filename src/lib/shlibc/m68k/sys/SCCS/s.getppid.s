h21033
s 00013/00000/00000
d D 1.1 86/07/31 12:30:05 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
#	@(#)getppid.s	6.1 
# getppid -- get parent process ID

	file	"getppid.s"
	set	getpid%,20
	global	getppid

getppid:
	MCOUNT
	mov.l	&getpid%,%d0
	trap	&0
	mov.l	%d1,%d0
	rts
E 1
