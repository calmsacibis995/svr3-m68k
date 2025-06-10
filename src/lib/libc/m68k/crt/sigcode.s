#	@(#)sigcode.s	6.1 
	file	"sigcode.s"

# _sigcode: the interface routine to user's signal handlers.

# The kernel forces the entry to _sigcode with the stack thus:
#
#	----		# tabu area: the user's stack
#	long	PC	# user's program counter
#	short	CC,kind	# user's condition codes, stack format flag
#	long	arg	# second argument
#	long	signo	# first argument.  This is the top of the stack.

# _sigcode does this:
#
# 1)	save those registers that wouldn't be saved by the user's signal handler
#
# 2)	call _sigcall(signo, arg) to call the user's signal handler
#
# 3)	restore the registers
#
# 4)	Pop the stack so that the kernel's stuff is gone,
#	at the same time restoring the condition codes and pc.
#	This effects a return to the user's interrupted program.

	set	F,-20		# 5 longs
	set	S,-16		# offset to movm storage area
	set	M,0x0303	# a1-a0, d1-d0
	set	SIGBUS,10
	set	SIGSEGV,11
	set	CONT,2		# sysm68k call to continue after signal handler violations
	text
	global	_sigcode
	global	_sigcall

_sigcode:
	link	%a6,&F		# push a6; set a6 to sp; decrement sp by F
	movm.l	&M,S(%a6)	# save a1-a0, d1-d0
	mov.l	8(%a6),(%sp)	# "push" arg
	mov.l	4(%a6),-(%sp)	# push signo
	jsr	_sigcall	# call _sigcall(signo, arg)
	movm.l	S(%a6),&M	# restore a1-a0, d1-d0
	unlk	%a6		# restore a6, sp
	tst.w	8(%sp)		# look at the kind flag in the pad word
	beq.b	L%normal	# 0 means 'rtr' is usable
#
# at this point we need to call the kernel to perform instr cont for us
#
	add.w	&8,%sp		# pop signo, arg 
	mov.l   %d0,-(%sp)	# arg1 - old %d0 value
	mov.l	&CONT,-(%sp)	# cmd - instruction continue
	jsr	_sysm68k	# no return
L%normal:
	add.w	&10,%sp		# pop signo, arg and stack format word
	rtr			# pop CC into cc, pop PC into pc
