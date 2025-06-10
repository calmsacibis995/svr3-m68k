#	@(#)access.s	6.1 
# access(file, request)
#  test ability to access file in all indicated ways
#  1 - read
#  2 - write
#  4 - execute

	file	"access.s"
	set	access%,33
	global	access
	global	cerror%

access:
	MCOUNT
	mov.l	&access%,%d0
	trap	&0
	bcc.b 	noerror
	jmp 	cerror%
noerror:
	rts
