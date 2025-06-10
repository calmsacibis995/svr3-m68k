h25295
s 00019/00000/00000
d D 1.1 86/07/31 12:30:08 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
#	@(#)link.s	6.1 
# C library -- link

# error = link(old-file, new-file);

	file	"link.s"
	set	link%,9
	global	link
	global	cerror%

link:
	MCOUNT
	mov.l	&link%,%d0
	trap	&0
	bcc.b 	noerror
	jmp 	cerror%
noerror:
	mov.l	&0,%d0
	rts
E 1
