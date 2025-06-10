#	@(#)ustat.s	6.1 
# C library -- ustat
#
# int ustat (dev,buf)
# int dev;
# struct ustat *buf

	file	"ustat.s"
	set	utssys%,57
	set	ustat%,2
	global	ustat
	global	cerror%

ustat:
	MCOUNT
	lea.l	4(%sp),%a0
	mov.l	&ustat%,%d0
	mov.l	%d0,-(%sp)
	mov.l	(%a0)+,-(%sp)
	mov.l	(%a0),-(%sp)
	bsr.b	sys
	add.w	&12,%sp
	rts

sys:
	mov.l	&utssys%,%d0
	trap	&0
	bcc.b	noerror
	jmp	cerror%
noerror:
	mov.l	&0,%d0
	rts
