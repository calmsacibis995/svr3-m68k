h22186
s 00016/00000/00000
d D 1.1 86/07/31 12:30:02 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
#	@(#)fcntl.s	6.1 
# C library -- fcntl

	file	"fcntl.s"
	set	fcntl%,62
	global	fcntl
	global	cerror%

fcntl:
	MCOUNT
	mov.l	&fcntl%,%d0
	trap	&0
	bcc.b	noerror
	jmp	cerror%
noerror:
	rts
E 1
