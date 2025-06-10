h27195
s 00019/00000/00000
d D 1.1 86/07/31 12:30:19 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
#	@(#)stime.s	6.1 
#  C - library stime

	file	"stime.s"
	set	stime%,25
	global	stime
	global  cerror%

stime:
	MCOUNT
	mov.l	4(%sp),%a0		# copy time to set
	mov.l	(%a0),4(%sp)
	mov.l	&stime%,%d0
	trap	&0
	bcc.b 	noerror
	jmp 	cerror%
noerror:
	mov.l	&0,%d0
	rts
E 1
