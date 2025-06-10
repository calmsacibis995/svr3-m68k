h23510
s 00016/00000/00000
d D 1.1 86/07/31 12:29:54 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
#	@(#)_sysm68k.s	6.1	
# C library - sysm68k

	file	"_sysm68k.s"
	set	sysm68k%,38
	global	_sysm68k
	global	cerror%

_sysm68k:
	MCOUNT
	mov.l	&sysm68k%,%d0
	trap	&0
	bcc.b	noerror
	jmp	cerror%
noerror:
	rts
E 1
