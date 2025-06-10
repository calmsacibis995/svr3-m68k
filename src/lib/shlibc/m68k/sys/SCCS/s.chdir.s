h25100
s 00019/00000/00000
d D 1.1 86/07/31 12:29:56 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
#	@(#)chdir.s	6.1 
# C library -- chdir

# error = chdir(string);

	file	"chdir.s"
	set	chdir%,12
	global	chdir
	global	cerror%

chdir:
	MCOUNT
	mov.l	&chdir%,%d0
	trap	&0
	bcc.b 	noerror
	jmp 	cerror%
noerror:
	mov.l	&0,%d0
	rts
E 1
