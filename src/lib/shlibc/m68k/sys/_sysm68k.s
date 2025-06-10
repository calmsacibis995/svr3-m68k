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
