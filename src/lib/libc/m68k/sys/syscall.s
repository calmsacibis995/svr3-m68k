#	@(#)syscall.s	6.1 
# C library -- syscall

#  Interpret a given system call

	file	"syscall.s"
	global	syscall
	global	cerror%

syscall:
	MCOUNT
	mov.l	4(%sp),%d0	# set up syscall number
	mov.l	(%sp)+,(%sp)	# one fewer arguments, but save rtn addr
	trap	&0
	bcc.b	noerror		# same number args as it stacked
	mov.l	(%sp),-(%sp)	# restore rtn addr, as caller will remove
	jmp	cerror%
noerror:
	mov.l	(%sp),-(%sp)	# restore rtn addr, as caller will remove
	rts
