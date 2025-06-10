h27447
s 00021/00000/00000
d D 1.1 86/07/31 12:30:03 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
#	@(#)fstat.s	6.1 
# C library -- fstat

# error = fstat(file, statbuf);

# char statbuf[34]

	file	"fstat.s"
	set	fstat%,28
	global	fstat
	global	cerror%

fstat:
	MCOUNT
	mov.l	&fstat%,%d0
	trap	&0
	bcc.b 	noerror
	jmp 	cerror%
noerror:
	mov.l	&0,%d0
	rts
E 1
