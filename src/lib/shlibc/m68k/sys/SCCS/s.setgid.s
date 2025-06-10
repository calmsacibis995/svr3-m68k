h25753
s 00019/00000/00000
d D 1.1 86/07/31 12:30:16 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
#	@(#)setgid.s	6.1 
# C library -- setgid

# error = setgid(uid);

	file	"setgid.s"
	set	setgid%,46
	global	setgid
	global  cerror%

setgid:
	MCOUNT
	mov.l	&setgid%,%d0
	trap	&0
	bcc.b 	noerror
	jmp 	cerror%
noerror:
	mov.l	&0,%d0
	rts
E 1
