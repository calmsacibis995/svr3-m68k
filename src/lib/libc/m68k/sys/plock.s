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
