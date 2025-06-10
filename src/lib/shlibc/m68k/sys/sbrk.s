#	@(#)sbrk.s	6.1 
#  C-library sbrk

#  oldend = sbrk(increment);

#  sbrk gets increment more core, and returns a pointer
#	to the beginning of the new core area

	file	"sbrk.s"
	set	break%,17
	global	sbrk
	global	end
	global	cerror%

sbrk:
	MCOUNT
	mov.l	nd,%d0
	add.l	%d0,4(%sp)
	mov.l	&break%,%d0
	trap	&0
	bcc.b 	noerr1
	jmp 	cerror%
noerr1:
	mov.l	nd,%d0
	mov.l	%d0,%a0		# fix: SGS compiler expects addr retval in %a0
	mov.l	4(%sp),nd
	rts

#  brk(value)
#  as described in man2.
#  returns 0 for ok, -1 for error.

	global	brk

brk:
	MCOUNT
	mov.l	&break%,%d0
	trap	&0
	bcc.b 	noerr2
	jmp 	cerror%
noerr2:
	mov.l	4(%sp),nd
	mov.l	&0,%d0
	rts

	data
nd:	
	long	end
