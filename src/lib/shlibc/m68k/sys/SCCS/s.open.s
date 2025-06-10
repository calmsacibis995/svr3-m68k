h25779
s 00020/00000/00000
d D 1.1 86/07/31 12:30:12 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
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
E 1
