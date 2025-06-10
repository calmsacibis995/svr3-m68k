h27141
s 00016/00000/00000
d D 1.1 86/07/31 12:30:01 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
#	@(#)exit.s	6.1 
# C library -- exit

# exit(code)
# code is %deturn in r0 to system
# Same as plain exit, for user who want to define their own exit.

	file	"exit.s"
	set	exit%,1
	global	_exit

_exit:
	MCOUNT
	mov.l	&exit%,%d0
	trap	&0
	stop	&0
E 1
