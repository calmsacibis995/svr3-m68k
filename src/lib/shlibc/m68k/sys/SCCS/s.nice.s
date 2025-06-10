h23445
s 00018/00000/00000
d D 1.1 86/07/31 12:30:11 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
#	@(#)nice.s	6.1 
# C library-- nice

# error = nice(hownice)

	file	"nice.s"
	set	nice%,34
	global	nice
	global  cerror%

nice:
	MCOUNT
	mov.l	&nice%,%d0
	trap	&0
	bcc.b 	noerror
	jmp 	cerror%
noerror:
	rts
E 1
