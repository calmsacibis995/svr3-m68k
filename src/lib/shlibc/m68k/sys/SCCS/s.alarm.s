h17611
s 00012/00000/00000
d D 1.1 86/07/31 12:29:56 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
#	@(#)alarm.s	6.1 
# C library - alarm

	file	"alarm.s"
	set	alarm%,27
	global	alarm

alarm:
	MCOUNT
	mov.l	&alarm%,%d0
	trap	&0
	rts
E 1
