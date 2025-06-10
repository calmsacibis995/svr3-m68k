h23605
s 00019/00000/00000
d D 1.1 86/07/31 12:30:00 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
#	@(#)dup.s	6.1 
# C library -- dup

#	f = dup(of [ ,nf])
#	f == -1 for error

	file	"dup.s"
	set	dup%,41
	global	dup
	global	cerror%

dup:
	MCOUNT
	mov.l	&dup%,%d0
	trap	&0
	bcc.b 	noerror
	jmp 	cerror%
noerror:
	rts
E 1
