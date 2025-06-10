#	@(#)utime.s	6.1 
# C library -- utime
 
#  error = utime(string,timev);
 
	file	"utime.s"
	set	utime%,30
	global	utime
	global	cerror%
 
utime:
	MCOUNT
	mov.l	&utime%,%d0
	trap	&0
	bcc.b	noerror
	jmp	cerror%
noerror:
	rts
