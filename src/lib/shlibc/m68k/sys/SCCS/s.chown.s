h25904
s 00019/00000/00000
d D 1.1 86/07/31 12:29:57 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
#	@(#)chown.s	6.1 
# C library -- chown

# error = chown(string, owner);

	file	"chown.s"
	set	chown%,16
	global	chown
	global	cerror%

chown:
	MCOUNT
	mov.l	&chown%,%d0
	trap	&0
	bcc.b 	noerror
	jmp 	cerror%
noerror:
	mov.l	&0,%d0
	rts
E 1
