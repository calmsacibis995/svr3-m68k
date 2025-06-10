#	@(#)open.s	6.1 
# C library -- open

# file = open(string, mode)

# file == -1 means error

	file	"open.s"
	set	open%,5
	global	open
	global  cerror%

open:
	MCOUNT
	mov.l	&open%,%d0
	trap	&0
	bcc.b 	noerror
	jmp 	cerror%
noerror:
	rts
