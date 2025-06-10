h26221
s 00019/00000/00000
d D 1.1 86/07/31 12:30:25 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
#	@(#)unlink.s	6.1 
# C library -- unlink

# error = unlink(string);

	file	"unlink.s"
	set	unlink%,10
	global	unlink
	global  cerror%

unlink:
	MCOUNT
	mov.l	&unlink%,%d0
	trap	&0
	bcc.b 	noerror
	jmp 	cerror%
noerror:
	mov.l	&0,%d0
	rts
E 1
