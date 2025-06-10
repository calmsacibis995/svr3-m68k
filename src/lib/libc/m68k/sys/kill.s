#	@(#)kill.s	6.1 
# C library -- kill

	file	"kill.s"
	set	kill%,37
	global	kill
	global cerror%

kill:
	MCOUNT
	mov.l	&kill%,%d0
	trap	&0
	bcc.b 	noerror
	jmp 	cerror%
noerror:
	mov.l	&0,%d0
	rts
