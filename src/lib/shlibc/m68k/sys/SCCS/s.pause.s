h23480
s 00018/00000/00000
d D 1.1 86/07/31 12:30:12 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
#	@(#)pause.s	6.1 
# C library - pause

	file	"pause.s"
	set	pause%,29
	global	pause
	global	cerror%

	even
pause:
	MCOUNT
	mov.l	&pause%,%d0
	trap	&0
	bcc.b	noerror
	jmp	cerror%
noerror:
	mov.l	&0,%d0
	rts
E 1
