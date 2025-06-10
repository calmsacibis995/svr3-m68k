h24219
s 00017/00000/00000
d D 1.1 86/07/31 12:16:51 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
#	@(#)cuexit.s	6.1 
# C library -- exit

# exit(code)
# code is return in r0 to system

	file	"cuexit.s"
	set	exit%,1
	global	exit
	global	_cleanup

exit:
	MCOUNT
	jsr	_cleanup
	mov.l	&exit%,%d0
	trap	&0
	stop	&0
E 1
