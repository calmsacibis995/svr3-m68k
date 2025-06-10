h24939
s 00019/00000/00000
d D 1.1 86/07/31 12:29:59 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
#	@(#)close.s	6.1 
# C library -- close

# error =  close(file);

	file	"close.s"
	set	close%,6
	global	close
	global	cerror%

close:
	MCOUNT
	mov.l	&close%,%d0
	trap	&0
	bcc.b 	noerror
	jmp 	cerror%
noerror:
	mov.l	&0,%d0
	rts
E 1
