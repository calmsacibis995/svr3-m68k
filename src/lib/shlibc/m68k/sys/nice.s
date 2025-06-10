#	@(#)nice.s	6.1 
# C library-- nice

# error = nice(hownice)

	file	"nice.s"
	set	nice%,34
	global	nice
	global  cerror%

nice:
	MCOUNT
	mov.l	&nice%,%d0
	trap	&0
	bcc.b 	noerror
	jmp 	cerror%
noerror:
	rts
