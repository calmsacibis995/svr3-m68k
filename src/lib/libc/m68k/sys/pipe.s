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
