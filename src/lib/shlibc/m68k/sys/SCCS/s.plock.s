h23812
s 00018/00000/00000
d D 1.1 86/07/31 12:30:13 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
#	@(#)plock.s	6.1 
# C library -- plock

# error = plock(op)

	file	"plock.s"
	set	plock%,45
	global	plock
	global	cerror%

plock:
	MCOUNT
	mov.l	&plock%,%d0
	trap	&0
	bcc.b	noerror
	jmp	cerror%
noerror:
	rts
E 1
