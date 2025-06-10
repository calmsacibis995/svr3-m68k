#	@(#)uname.s	6.1 
# C library -- uname
#
# int uname (name)
# struct utsname *name

	file	"uname.s"
	set	utssys%,57
	set	uname%,0
	global	uname
	global	cerror%

uname:
	MCOUNT
	mov.l	4(%sp),%d0
	mov.l	&uname%,%d1
	mov.l	%d1,-(%sp)
	sub.l	&4,%sp		# dummy arg for fill
	mov.l	%d0,-(%sp)
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
