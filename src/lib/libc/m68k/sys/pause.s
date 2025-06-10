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
