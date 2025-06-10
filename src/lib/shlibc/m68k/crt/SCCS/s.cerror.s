h23957
s 00017/00000/00000
d D 1.1 86/07/31 11:59:56 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
#	@(#)cerror.s	6.1 
# C return sequence which
# sets errno, returns -1.

	file	"cerror.s"
	global	cerror%
	global	errno

	data
errno:	long	0

	text
cerror%:
	mov.l	%d0,errno
	mov.l	&-1,%d0
	mov.l	%d0,%a0
	rts
E 1
