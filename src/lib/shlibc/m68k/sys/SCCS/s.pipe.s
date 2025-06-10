h27103
s 00023/00000/00000
d D 1.1 86/07/31 12:30:13 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
#	@(#)pipe.s	6.1 
# pipe -- C library

#	pipe(f)
#	int f[2];

	file	"pipe.s"
	set	pipe%,42
	global	pipe
	global  cerror%

pipe:
	MCOUNT
	mov.l	&pipe%,%d0
	trap	&0
	bcc.b 	noerror
	jmp 	cerror%
noerror:
	mov.l	4(%sp),%a0
	mov.l	%d0,(%a0)+
	mov.l	%d1,(%a0)
	mov.l	&0,%d0
	rts
E 1
