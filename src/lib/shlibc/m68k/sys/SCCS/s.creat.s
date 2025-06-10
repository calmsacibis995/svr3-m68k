h26222
s 00019/00000/00000
d D 1.1 86/07/31 12:29:59 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
#	@(#)creat.s	6.1 
# C library -- creat

# file = creat(string, mode);
# file == -1 if error

	file	"creat.s"
	set	creat%,8
	global	creat
	global	cerror%

creat:
	MCOUNT
	mov.l	&creat%,%d0
	trap	&0
	bcc.b 	noerror
	jmp 	cerror%
noerror:
	rts
E 1
