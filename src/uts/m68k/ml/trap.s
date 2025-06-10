#####################################################################
#		File:  trap.s
#
#  This package provides exception handling for Unix.  Basically,
#  exceptions are divided into one of several basic types:
#
#  1)  Interrupts (Notably the clock)
#  2)  System calls.
#  3)  Traps (divide by 0, illegal instructions, etc.)
#  4)  Bus Errors (handled in paging.s)
#
#####################################################################

set	SAVEREGS,0xFFFE		# MOVM pattern for saving all except sp.
set	GETREGS,0x7FFF		# MOVM pattern for getting all except sp.

#####################################################################
# Clock interrupt
#
#   Control comes here whenever a clock interrupt occurs.
#   We need to call clock as follows:
#
#	clock(pc, ps);
#
#   The ps must be the ps of the machine before the interrupt.
#	This is not true on a throw-away stack frame.
#####################################################################
	text
	global	Xclock
Xclock:
	movm.l	&0xc0c0,-(%sp)		# Save the scratch registers.
	mov.w	0x10(%sp),%d1		# Save current status register.
	mov.w	0x16(%sp),%d0		# Type/offset from interrupt stack
	lsr.w	&12,%d0			# shift off vector offset, leave type
	beq.b	normal			# If normal int frame, use above sr.
					# Throw away stack frame!
	or.w	&0x1000,%sr		# switch back to master stack
	mov.w	(%sp),%d1		# get previous status register
	and.w	&0xefff,%sr		# switch back to interrupt stack
normal:
	clr.l	-(%sp)			# Make room for sr on the stack
	mov.w	%d1,2(%sp)		# push sr as arg to clock
	mov.l	16+4+2(%sp),-(%sp)	# PC on the stack.

	jsr	clock			# Call the clock routine.

	add.l	&8,%sp			# Pull SR and PC off.
	tst.l	%d0			# Test return value from clock
	beq.b	intret			# Normal interrupt return

#####################################################################
#
# At this point, clock returned a non-zero value and we need to run the user
# profiling (addupc).  Clock could not run user profiling because it was
# executing on the interrupt stack and addupc can get a page fault and sleep.
# We were previously in user mode before the interrupt, therefore we are the 
# only interrupt on the stack.  To run the user profiling, we must clean up the
# interrupt stack, switch back to the master stack and lower the interrupt
# level before executing addupc(pc, &u.u_prof, 1).
#
#####################################################################

	movm.l	(%sp)+,&0x303		# Restore the scratch registers
	add.l	&0x8,%sp		# Bump past the sr/pc/vector
	or.w	&0x1000,%sr		# Switch back to the master stack
	and.w	&0x3000,%sr		# lower the interrupt level to zero

	movm.l	&0xc0c0,-(%sp)		# Save the scratch registers again.
	mov.l	&1,-(%sp)		# push tick (1) argument
	mov.l	&u+u_prof,-(%sp)	# push u.u_prof address
	mov.l	26(%sp),-(%sp)		# Put the pc on the stack.

	jsr	addupc			# Do the dirty deed

	add.l	&12,%sp			# Pop off args
	movm.l	(%sp)+,&0x303		# Restore the scratch registers
	bra.b	resched			# See if rescheduling is needed

#####################################################################
#
# Common interrupt return for all interrupt routines
#
# Interrupts use the interrupt stack.  It is important that this
# stack gets cleaned up if we are going to switch to another process.
# Process switching can only occur on the master stack.  If we have 
# nested interrupts (normal stack frame), then don't process switch.
# Just return from interrupt to the previous interrupt.
#
#####################################################################

	global	intret
intret:
	mov.w	0x16(%sp),%d0	# Type/offset from interrupt stack
	lsr.w	&12,%d0		# shift off vector offset, leave type
	movm.l	(%sp)+,&0x303	# Get back the saved registers
	beq.b	nosched		# If normal interrupt frame, return from int.
				# Another interrupt is underneath this one.

# At this point, we know we were previously using the master stack 
# and the real stack frame is on that stack.  We are going to clean
# up the interrupt stack entirely and switch back to the master stack.

	add.l	&0x8,%sp	# Bump past the sr/pc/vector
	or.w	&0x1000,%sr	# Switch back to the other stack

	global	resched
resched:			# Determine if rescheduling is neccessary.
	btst	&5,(%sp)	# If interrupted in system mode,
	bne.b	nosched		# then don't reschedule.
	tst.b	runrun		# If runrun is clear, then don't
	beq.b	nosched		# reschedule.
#
# Rescheduling is needed if control comes here.
# We need to call trap with 256 for vecnum to reschedule.
#
	mov.l	2(%sp),-(%sp)	# Put the pc on the stack.
	clr.l	-(%sp)		# Make a space for status.
	mov.w	&256,(%sp)	# Put vector number in status.
	mov.w	8(%sp),2(%sp)	# Put status register in status.
	movm.l	&0xFFFE,-(%sp)	# Pass all user registers.
	mov.l	%usp,%a0	# Save usp on stack
	mov.l	%a0,-(%sp)

	jsr	trap		# Call trap to reschedule it.

	mov.l	(%sp)+,%a0	# restore the usp
	mov.l	%a0,%usp
	movm.l	(%sp)+,&0x7fff	# Recover registers and return.
	mov.w	2(%sp),8(%sp)	# Copy back the status register
	mov.l	4(%sp),10(%sp)	# Copy back the pc
	add.l	&8,%sp		# Bump past the sr/pc
nosched:
	rte			# return to interrupted process


#####################################################################
#
#			Xtrap - All trap calls
#
#  trap(usp,d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,status,pc);
#	All arguments are long, aligned on 4 byte boundary.
#
#  Control comes here on any exception (addr errors, illegal op., 0 divide etc)
#  Prepare parameter list for trap, it will do the rest.
#  Note that extra lengths are gone to to always maintain the stack 
#  aligned on 4 byte boundaries, so be cautious about making changes. 
#
#####################################################################

nullvect:	global	nullvect
alltraps:	global	alltraps
Xtrap:		global	Xtrap

	mov.l	2(%sp),-(%sp)		# Put the pc on the stack.

	clr.l	-(%sp)			# Make a space for status.
	mov.w	4+4+6(%sp),(%sp)	# Put vector number in status.
	and.w	&0xfff,(%sp)		# Clear format bits if any.
	mov.w	4+4(%sp),2(%sp)		# Put status register in status.

	movm.l	&SAVEREGS,-(%sp)	# Pass all user registers.
	mov.l	%usp,%a0		# Include the usp.
	mov.l	%a0,-(%sp)

	jsr	trap			# Finished w/ parameters, make the call.

	lea.l	76(%sp),%a2		#  Address of type/offset long + 1/2 pc
	mov.l	(%a2),%d0		#  Type/offset of frame to be saved
	lsr.w	&12,%d0			#  Shift out vector offset, leave type
	mov.b	(stkfmtsz,%d0.w),%d0	#  Using type; lookup frame size (bytes)

	mov.l	%a2,%a1			#  stack frame address
	add.w	%d0,%a1			#  End of stack frame
	clr.l	(%a1)			#  New Type/offset is zero
	mov.w	&18,%d1			#  15 regs, usp, 4 wd frame, pc/status
L%dlp2:	mov.l	-(%a2),-(%a1)		#  Move registers, usp, status, pc and
	dbra.w	%d1,L%dlp2		#  short frame to end of kernel stack.
	add.w	%d0,%sp			#  Adjust stack by size of stack frame

	mov.l	(%sp)+,%a0		# Restore the users
	mov.l	%a0,%usp		# stack pointer;
	movm.l	(%sp)+,&GETREGS		# and all registers.
	mov.w	2(%sp),4+4(%sp)		# Copy back the status register
	mov.l	4(%sp),4+4+2(%sp)	# Copy back the pc
	add.l	&8,%sp			# Zap status, and pc.
	rte

#####################################################################
#
#		Xsyscall - All system calls
#
# syscall(usp,d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,status,pc);
#	All arguments are long, aligned on 4 byte boundary.
#
#  Control comes here to process a system call.
#  A system call is generated by a trap #0 instruction.
#  Basically, all we have to do is call syscall() with its
#  parameter list and let it handle the details.
#  Note that extra lengths are gone to to always maintain the stack 
#  aligned on 4 byte boundaries, so be cautious about making changes. 
#
#####################################################################
	global	Xsyscall
	global	fixprr
Xsyscall:
	mov.l	2(%sp),-(%sp)		#  Put the pc on the stack.

	clr.l	-(%sp)			#  Make a space for status.
	mov.w	4+4+6(%sp),(%sp)	#  Put vector number in status.
	and.w	&0xfff,(%sp)		#  Clear format bits if any.
	mov.w	4+4(%sp),2(%sp)		#  Put status register in status.

	movm.l	&SAVEREGS,-(%sp)	#  Save the registers.
	mov.l	%usp,%a0		#  Including the usp.
	mov.l	%a0,-(%sp)

	jsr	syscall			#  The parameters are built, now do it.

	mov.l	saveflag,%a0		#  Restore stack frame flag address
	tst.b	(%a0)			#  get the flag itself
	beq.b	L%skip2			#  If zero, nothing to do.
					#  else restore the stack frame
	clr.b	(%a0)			#  Reset the save-flag
	mov.l	saveaddr,%a0		#  Get address of stack frame save area
	mov.l	(%a0),%d1		#  Get stack frame type/vector offset
	lsr.w	&12,%d1			#  Shift out vector offset, leave type
	mov.b	(stkfmtsz,%d1.w),%d1	#  Using type, lookup frame size (bytes)
	mov.l	%sp,%a2			#  Load current kernel stack pointer
	sub.w	%d1,%sp			#  Subtract frame size to make room
	mov.l	%sp,%a1			#  Save result, this is new top of stack

	mov.w	&18,%d0			#  (15 regs+usp)*4 + (4 wd frame)*2 + 8
L%dlp3:	mov.l	(%a2)+,(%a1)+		#  Move registers to make room for frame
	dbra.w	%d0,L%dlp3		#  New frame is place at end of stack

	lsr.w	&2,%d1			#  Convert bytes to longs
L%dlp4:	mov.l	(%a0),(%a1)+		#  Move saved frame onto kernel stack
	clr.l	(%a0)+			#  Clear the save buffer (invalidated)
	dbra.w	%d1,L%dlp4		#  This should abut end of U-block

	btst	&2,9+72(%sp)		# Check PRR bit. 
	bne.b	L%skip2			# If 1, no fixup is needed.
					#  This goes away on good 020 chip.
	lea.l	72(%sp),%a2		#  pointer to stack frame
	bsr.l	fixprr			#  Fix PRR per errata sheet in buserr.

L%skip2: mov.l	(%sp)+,%a0		#  Restore the user's
	mov.l	%a0,%usp		#  stack pointer.
	movm.l	(%sp)+,&GETREGS		#  Restore all user registers.

	mov.w	2(%sp),4+4(%sp)		#  Copy back the status register
	mov.l	4(%sp),4+4+2(%sp)	#  Copy back the pc
	add.l	&8,%sp			#  Undo status/pc save.
	rte				#  We're done, bye.

#####################################################################
