#	@(#)alarm.s	6.1 
# C library - alarm

	file	"alarm.s"
	set	alarm%,27
	global	alarm

alarm:
	MCOUNT
	mov.l	&alarm%,%d0
	trap	&0
	rts
