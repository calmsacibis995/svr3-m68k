h25609
s 00019/00000/00000
d D 1.1 86/07/31 12:29:57 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
#	@(#)chmod.s	6.1 
# C library -- chmod

# error = chmod(string, mode);

	file	"chmod.s"
	set	chmod%,15
	global	chmod
	global	cerror%

chmod:
	MCOUNT
	mov.l	&chmod%,%d0
	trap	&0
	bcc.b 	noerror
	jmp 	cerror%
noerror:
	mov.l	&0,%d0
	rts
E 1
