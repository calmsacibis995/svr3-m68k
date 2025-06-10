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
