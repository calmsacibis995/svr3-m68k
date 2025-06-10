h30508
s 00020/00000/00000
d D 1.1 86/07/31 12:29:55 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
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
E 1
