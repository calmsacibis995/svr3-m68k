#		Copyright (c) 1985 AT&T		#
#		All Rights Reserved		#
#	@(#)psl.m	1.0	
#
#	processor status register for 68020
#

	set	PS_C,0x0001		# carry bit 
	set	PS_V,0x0002		# overflow bit 
	set	PS_Z,0x0004		# zero bit 
	set	PS_N,0x0008		# negative bit 
	set	PS_EXT,0x0010		# extend bit 

	set	PS_IPL,0x0700		# interrupt priority level 
	set	PS_M,0x1000		# Master/interrupt state 
	set	PS_CUR,0x2000		# current mode. Supervisor if set 
	set	PS_T0,0x4000		# trace enable bit 0 
	set	PS_T1,0x8000		# trace enable bit 1 
	set	PS_T,PS_T1		# trace enable bit 1 

#		Copyright (c) 1985 AT&T		#
#		All Rights Reserved		#
#	@(#)equates.m	1.0	

#  General equate file.
#  This file contains equates which are used throughout the machine
# language routines.  Any equate of this nature should be placed in
# this file.  Equates that are specific to a single machine language
# source file should be defined in the source file that contains them.

set	allfpi,0xff		# Floating point register save/restore mask.
set	savmsk,0xfcff		# this mask determines which registers are saved
set	regsize,14*4		# Size of mem required to save them.
				#  Currently this mask causes D0..D7 and A2..A7.

set	u,0x1000000		# Address of U-block
	global u		# Define the U-block for the entire system

#
#  Cache control functions.
#

set	CACHDIS,8		# Cache diable and flush
set	CACHENB,9		# Cache enable and flush

set	BTOL,2			# Number of shifts for bytes to longs.

#  Ublock equates.  The following may need changing when the user.h
# file is changed.

set	u_procp,0x1bc
set	u_caddrflt,0x70
set	u_prof,0x248

#  Proc equates.  The following may need changing when proc.h changes.
#

set	p_sysid,0x64
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
#		Copyright (c) 1985 AT&T		#
#		All Rights Reserved		#
#	@(#)mmb.s	6.10	
#
# mmbstore(reg, val)		/* store to mmu register */
# register char *reg;	
# register int val;	
	data
	global mmbad
mmbad:	long 0x10000
	text
	global	mmbstore
mmbstore:
	link	%fp,&0
	tst.w	do_pmmu
	bne	pmmustore
	mov.l	8(%fp),%a1
	mov.l	12(%fp),%d1
#
	add.l	mmbad,%a1	# MMU access address
	mov.l	%dfc,%a0
	mov.l	&7,%d0		# CPU space function code
	mov.l	%d0,%dfc
	movs.l	%d1,(%a1)	# move to MMU register 
	mov.l	%a0,%dfc
L%189:
	unlk	%fp
	rts

pmmustore:
	mov.l	12(%fp),%d1	# Get parameter "val"
	mov.l	8(%fp),%d0	#Get "reg" in d0.

	beq	rpreg		# Should we write TC register?
	nop
	short	0xf001		# "pmove.l %d1,tc"
	short	0x4000
	bra	mmbrts

rpreg:
	mov.l	%d1,rpstore+4	# Write root pointer.
	
	nop
	short	0xf039		# "pmove rpstore,crp"
	short	0x4c00
	long	rpstore
	nop
	short	0xf000		# "pflusha"
	short	0x2400

mmbrts:
	unlk	%fp
	rts

#  This is a work around for mask m00a99a.
	data

trp:
	long	0x7fff0002
	long	0x05010

	text

	global	wakemmu
wakemmu:
	tst.w	do_pmmu
	beq	wakeret
	movm.l	&0x0340,-(%sp)		# Save registers.

	mov.l	&7,%d7
	mov.l	%d7,%dfc
	mov.l	%d7,%sfc
	clr.l	%d6
	movs.l	%d6,0x20010

	movs.w	%d6,0x20002

	lea	trp,%a1

	nop
	long	0xf0114c00

	nop
	short	0xf127
	mov.l	&0x2211,0x18(%a7)
	mov.l	0x1c(%a7),%d6

	and.l	&0xff100380,%d6
	or.l	&0x006e5420,%d6
	mov.l	%d6,0x1c(%a7)

	nop
	short	0xf15f

	movs.w	0x20000,%d6

	nop
	long	0xf0072400

	nop
	short	0xf127

	mov.l	&0x4c00,0x18(%a7)
	mov.l	0x1c(%a7),%d6
	and.l	&0xff300380,%d6
	or.l	&0x00425020,%d6
	mov.l	%d6,0x1c(%a7)

	nop
	short	0xf15f

	movs.w	0x20000,%d6

	lea	trp,%a1

	nop
	long	0xf0116000

	mov.l	&UD,%d7
	mov.l	%d7,%sfc
	mov.l	%d7,%dfc

	movm.l	(%sp)+,&0x02c0
wakeret:
	rts


#
#  The following routine sets the global flag "do_pmmu" to indicate
# whether or not a PMMU was found.
chk_mmu: global	chk_mmu

	mov.l	%vbr,%a0	# Save F line handler
	mov.l	0x2C(%a0),-(%sp)
	mov.l	&chk_mmu6,0x2c(%a0)

#	pflusha
	short	0xf000
	short	0x2400

	mov.w	&1,do_pmmu
chk_mmu4:
	mov.l	(%sp)+,0x2c(%a0)	# Restore old handler.
	rts

chk_mmu6:
	clr.w	do_pmmu
	mov.l	&chk_mmu4,2(%sp)
	rte

	data
	global	do_pmmu
do_pmmu:
	short	0		# Set to indicate pmmu present.

	global	rpstore
rpstore:
	long	0x7fff0003
	long	0
	
	text

# @(#)paging.s	1.2
#	Protection  fault
	global	Xprotflt
Xprotflt:
#
#  Control comes here when a bus error occurs.  This can be either due
# to an access of a faulted page or a write access of a write protected
# page.
#
#  At this point, the first few words of the stack look like:
#  (Refer to 68020 users manual for any additional details needed.)
#
#	| Fault addr lo	|
#	-----------------
#	| Fault addr hi	|  <- valid for data faults only.
#	-----------------
#	| Pipe B	|  <- Pipe B and C are code faults.
#	-----------------
#	| Pipe C	|
#	-----------------
#	| Special stat	|
#	-----------------
#	| Internal reg	|
#	-----------------
#	| BERR vector	|
#	-----------------
#	| PC lo		|
#	-----------------
#	| PC hi		|
#	-----------------
# SP->	| Status reg.	|
#	-----------------
#
#  Most of the work will actually be done by buserr().  buserr() must be
#  called with the following parameters:
#
#  	buserr(usp,r0,r1,r2, ... r14,status,pc);
#
#  buserr() returns a flag which if 0, indicates that the data is now
#  paged in and we should retry the access otherwise 
#  abort the access and PC contains the address to abort to.
#
#  Build the parameters to pass to buserr().
#
	mov.l	2(%sp),-(%sp)		#  Put the pc on the stack.
	clr.l	-(%sp)			#  Make a space for status.
	mov.w	4+4+6(%sp),(%sp)	#  Save the buserror vector number.
	mov.w	4+4(%sp),2(%sp)		#  Put status register on stack.
	movm.l	&SAVEREGS,-(%sp)	#  Save all the registers
	mov.l	%usp,%a0		#  including the User stack pointer.
	mov.l	%a0,-(%sp)
#
#  All the parameters to be passed are now on the stack, the next
#  thing to do is to call fault to handle the error.
#

callflt:
	jsr	buserr			# Go do it.

	tst.l	%d0			# Check the returned flag,
	beq.w	pagedin			# if its zero, then retry.
#
#  The page could not be paged in if control comes here.  We need to 
#  clear the stack before returning.  The stack frame may have to be saved
#  in the U-block.  In all cases, the frame must be removed and replaced
#  with a 4 word frame (type 0).
#
	lea.l	76(%sp),%a2		#  Address of type/offset long + 1/2 pc
	mov.l	(%a2),%d0		#  Type/offset of frame to be saved
	lsr.w	&12,%d0			#  Shift out vector offset, leave type
	mov.b	(stkfmtsz,%d0.w),%d0	#  Using type; lookup frame size (bytes)

	mov.l	saveflag,%a0		#  Save stack frame flag address
	tst.b	(%a0)			#  Is the flag set? 
	beq.b	L%skip3			#  If zero, nothing to do.
					#  else save the stack frame
	clr.b	(%a0)			#  Reset the save-flag
	mov.l	saveaddr,%a0		#  Get address of stack frame save area
	mov.l	%a2,%a1			#  Get address of stack frame
	mov.w	%d0,%d1			#  Get size of frame
	lsr.w	&2,%d1			#  Convert bytes to longs
L%dlp6:	mov.l	(%a1)+,(%a0)+		#  Move frame on stack to save area
	dbra.w	%d1,L%dlp6		#  in the U-block
#
# Clean up stack frame
#
L%skip3:
	mov.l	%a2,%a1			#  stack frame address
	add.w	%d0,%a1			#  End of stack frame
	clr.l	(%a1)			#  New Type/offset is zero
	mov.w	&18,%d1			#  15 regs, usp, 4 wd frame, pc/status
L%dlp7:	mov.l	-(%a2),-(%a1)		#  Move registers, usp, status, pc and
	dbra.w	%d1,L%dlp7		#  short frame to end of kernel stack.
	add.w	%d0,%sp			#  Adjust stack by size of stack frame

	mov.l	(%sp)+,%a0		#  Restore the user's
	mov.l	%a0,%usp		#  stack pointer.
	movm.l	(%sp)+,&GETREGS		#  Restore the user's registers.

	mov.w	2(%sp),8(%sp)		#  Copy back the status register.
	mov.l	4(%sp),10(%sp)		#  Copy back the PC.
	add.l	&8,%sp			#  Bump past the sr/pc.
	rte				#  Start at new pc.

#  The page was paged in so basically just rte to allow the process
#  to retry the access.
#
pagedin:
	btst	&2,9+72(%sp)		# Check PRR bit. 
	bne.b	nofixup			# If 1, no fixup is needed.

	lea.l	72(%sp),%a2		#  Fix PRR per errata sheet.
	bsr.b	fixprr			#  This goes away on good 020 chip.

nofixup:
	mov.l	(%sp)+,%a0		# Restore the user registers.
	mov.l	%a0,%usp
	movm.l	(%sp)+,&GETREGS
	add.l	&8,%sp			# bump past the vector.
	rte				# Retry the access.


#   The following code is a fixup for a bug in the 68020 chip as
#   outlined in the errata.  This code should be removed when the chip
#   is working correctly.
	global  fixprr
fixprr:
	btst	&0,0xA(%a2)	# Check DF bit.
	beq	noDF

	bclr	&0,0xA(%a2)	# Clear DF bit.

	mov.l	0x10(%a2),%a0	# Get address.
	btst	&6,0xB(%a2)	# Check for read or write.
	beq	dfwrite

	btst	&5,0xB(%a2)	# Check for words.
	beq	nodfrwds

	mov.l	0x10(%a2),%a0	# Get address.

	btst	&2,0xB(%a2)	# Check for supervisor space. 
	bne	dosup1
	mov.l	%a0,-(%sp)
	jsr	fushort
	add.l	&4,%sp

	bra	rdffixdone
dosup1:
	clr.l	%d0
	mov.w	(%a0),%d0
	bra	rdffixdone
nodfrwds:
	btst	&4,0xB(%a2)	# Check for byte.
	beq	nodfrbts

	mov.l	0x10(%a2),%a0	# Get address.

	btst	&2,0xB(%a2)	# Check for supervisor space. 
	bne	dosup2
	mov.l	%a0,-(%sp)
	jsr	fubyte
	add.l	&4,%sp
	bra	rdffixdone
dosup2:
	clr.l	%d0
	mov.b	(%a0),%d0
	bra	rdffixdone
nodfrbts:
	mov.l	0x10(%a2),%a0	# Get address.

	btst	&2,0xB(%a2)	# Check for supervisor space. 
	bne	dosup3
	mov.l	%a0,-(%sp)
	jsr	fuword
	add.l	&4,%sp
	bra	rdffixdone
dosup3:
	mov.l	(%a0),%d0
	
	
rdffixdone:
	mov.l	%d0,0x2C(%a2)	# Put in buffer.
	bra	dffixdone

dfwrite:

	mov.l	0x18(%a2),%d0	# Get data in d0.
	btst	&5,0xB(%a2)	# Check for words.
	beq	nodfwwds	
	mov.l	0x10(%a2),%a0	# Get address.
	mov.l	0x18(%a2),%d0	# Get data in d0.

	btst	&2,0xB(%a2)	# Check for supervisor space. 
	bne	dosup4

	mov.l	%d0,-(%sp)
	mov.l	%a0,-(%sp)
	jsr	sushort
	add.l	&8,%sp
	bra	dffixdone
dosup4:
	mov.w	%d0,(%a0)
	bra	dffixdone
nodfwwds:
	btst	&4,0xB(%a2)	# Check for byte.
	beq	nodfwbts
	mov.l	0x10(%a2),%a0	# Get address.
	mov.l	0x18(%a2),%d0	# Get data in d0.

	btst	&2,0xB(%a2)	# Check for supervisor space. 
	bne	dosup5

	mov.l	%d0,-(%sp)
	mov.l	%a0,-(%sp)
	jsr	subyte
	add.l	&8,%sp
	bra	dffixdone
dosup5:
	mov.b	%d0,(%a0)
	bra	dffixdone
nodfwbts:
	mov.l	0x10(%a2),%a0	# Get address.
	mov.l	0x18(%a2),%d0	# Get data in d0.

	btst	&2,0xB(%a2)	# Check for supervisor space. 
	bne	dosup6

	mov.l	%d0,-(%sp)
	mov.l	%a0,-(%sp)
	jsr	suword
	add.l	&8,%sp
	bra	dffixdone
	
dosup6:
	mov.l	%d0,(%a0)
dffixdone:

	
noDF:

	mov.l	0x14(%a2),%a0	# Get fixup address.


	btst	&1,8(%a2)	# Get mode
	beq	fixwrite	# If write.


	btst	&7,9(%a2)	# Check for words.
	beq	notrwds


	mov.l	0x14(%a2),%a0	# Get fixup address.

	btst	&5,0x9(%a2)	# Check for supervisor space. 
	bne	dosup7

	mov.l	%a0,-(%sp)
	jsr	fushort
	add.l	&4,%sp
	bra	rfixdone
dosup7:
	mov.w	(%a0),%d0
	bra	rfixdone
notrwds:
	btst	&6,9(%a2)	# Check for byte.
	beq	notrbts


	mov.l	0x14(%a2),%a0	# Get fixup address.

	btst	&5,0x9(%a2)	# Check for supervisor space. 
	bne	dosup8

	mov.l	%a0,-(%sp)
	jsr	fubyte
	add.l	&4,%sp
	bra	rfixdone
dosup8:
	mov.b	(%a0),%d0
	bra	rfixdone
notrbts:

	mov.l	0x14(%a2),%a0	# Get fixup address.

	btst	&5,0x9(%a2)	# Check for supervisor space. 
	bne	dosup9

	mov.l	%a0,-(%sp)
	jsr	fuword
	add.l	&4,%sp
	bra	rfixdone
dosup9:
	mov.l	(%a0),%d0
	
	
rfixdone:
	mov.l	%d0,0x2C(%a2)	# Put in buffer.
	bra	fixdone

fixwrite:


	mov.l	0x14(%a2),%a0	# Get fixup address.
	mov.l	0x18(%a2),%d0	# Get data in d0.
	btst	&7,9(%a2)	# Check for words.
	beq	notwwds	

	mov.l	0x14(%a2),%a0	# Get fixup address.
	mov.l	0x18(%a2),%d0	# Get data in d0.

	btst	&5,0x9(%a2)	# Check for supervisor space. 
	bne	dosup10

	mov.l	%d0,-(%sp)
	mov.l	%a0,-(%sp)
	jsr	sushort
	add.l	&8,%sp
	bra	fixdone
dosup10:
	mov.w	(%a0),%d0
	bra	fixdone
notwwds:
	btst	&6,9(%a2)	# Check for byte.
	beq	notwbts

	mov.l	0x14(%a2),%a0	# Get fixup address.
	mov.l	0x18(%a2),%d0	# Get data in d0.

	btst	&5,0x9(%a2)	# Check for supervisor space. 
	bne	dosup11

	mov.l	%d0,-(%sp)
	mov.l	%a0,-(%sp)
	jsr	subyte
	add.l	&8,%sp
	bra	fixdone
dosup11:
	mov.b	(%a0),%d0
	bra	fixdone
notwbts:

	mov.l	0x14(%a2),%a0	# Get fixup address.
	mov.l	0x18(%a2),%d0	# Get data in d0.

	btst	&5,0x9(%a2)	# Check for supervisor space. 
	bne	dosup12

	mov.l	%d0,-(%sp)
	mov.l	%a0,-(%sp)
	jsr	suword
	add.l	&8,%sp
	bra	fixdone
	
dosup12:
	mov.l	(%a0),%d0
fixdone:

	bset	&2,9(%a2)	# Set the PRR bit.
#  end of fixup.
	rts

	text
#		Copyright (c) 1985 AT&T		#
#		All Rights Reserved		#
#	@(#)start.m4	6.8	

# MC68020 version
# The code in this file is executed at system bootstrap.

	set	GRAIN,1024		# Debug - ifdefs don't work yet.
	set	ROUND,GRAIN-1		# granularity round
	set	GMASK,-GRAIN		# granularity mask
	set	STSIZE,2048		# boot and interrupt stack for system
	set	U_SIZE,4096		# U block for Proc 0
	set	HIPRI,0x2700		# supervisor state, ipl 7
	set	MSP,0x1000		# Master stack bit in the SR

	data	# these variables can't be in bss space
	global	physmem
physmem:	long	0		# total memory
vbr:		long	0		# contents of vector base register
	global	Dconcurrent
Dconcurrent:	long	0		# diagnostic concurrent mode enabled

bootdriv:	global bootdriv		# boot drive from %d0
		long 0
bootctlr:	global bootctlr		# boot controller from %d1
		long 0
bootinod:	global bootinod		# boot inode from %d2
		long 0
dumpdev:	long -1

	text
start:	global	start
_start:	global	_start
	mov.w	&HIPRI,%sr		# supervisor state, ipl 7
	mov.l	%d0,bootdriv		# save boot params
	mov.l	%d1,bootctlr
	mov.l	%d2,bootinod

	mov.l	%vbr,%a0		# get address of diagnostic VBR
	tst.l	(%a0)			# check for concurrency
	bne	_Dcon_off
	mov.l	&1,Dconcurrent		# concurrent ON
	bra.s	_Dcon_exit
_Dcon_off:
	mov.l	&0,Dconcurrent		# concurrent OFF
_Dcon_exit:

	mov.l	0x10,romillv

	mov.l	&M68Kvec,%d1		# set vector base register
	mov.l	%d1,%vbr
	mov.l	%d1,vbr			# save the contents of the vbr
	mov.l	&UD,%d1			# Setup address space registers.
	mov.l	%d1,%dfc
	mov.l	%d1,%sfc

	mov.l	&CACHENB,%d1		# Enable the on-chip cache.
	mov.l	%d1,cacr
	mov.l	%d1,%cacr

	mov.l	&0xfffc0000,%a0		# Enable the board level cache.
	mov.b	&0xf1,(%a0)

	tst.w	%d0			# check for valid boot device
	bmi.b	L%strt
# Look up boot devices parameters.
	mov.l	&sysdevs,%a0		# get address of system devices table
	mov.w	(%a0),%d1		# get nbr of system devices (or -1)
	mov.w	&-1,(%a0)+		# force single execution only
	cmp.w	%d0,%d1
	bge.b	L%strt			# bra if either already switched or none
	lsl.w	&4,%d0			# 16 bytes per device
	add.w	%d0,%a0
	mov.l	bootctlr,%d1		# Get controller number.
	lsl.w	&6,%d1			# 64 bytes per controller.
	add.w	%d1,%a0
	mov.w	(%a0)+,rootdev		# move system device info into unix
	mov.w	(%a0)+,pipedev
	mov.w	(%a0)+,dumpdev
	mov.w	(%a0)+,swapdev
	mov.l	(%a0)+,swplo
	mov.l	(%a0),nswap
# Clear Unix's BSS area including temporary u block.

L%strt:	mov.l	&end+ROUND,%d0		# top of unix
	and.l	&GMASK,%d0
	add.l	&STSIZE,%d0		# for boot stack area

	mov.l	&edata,%a0		# zero bss and boot stack
	mov.l	&0,%d7			# constant 0
L%clr:	mov.l	%d7,(%a0)+		# assumes edata is long aligned
	cmp.l	%a0,%d0
	blt.b	L%clr

# Call C routines to startup system.

	mov.l	%d0,%sp			# use boot stack area until mmu ready
					# afterwards, use area as interrupt stk
	mov.l	%d0,-(%sp)		# pass start of ram address to mlsetup
	jsr	mlsetup			# mlsetup()
	add.l	&4,%sp			# pop the stack
	or.w	&MSP,%sr		# Enable master stack pointer, set this
	mov.l	&u+U_SIZE-4,%sp		# to system stack, at top of u area
					# (-4) beware of printf argument loading
	jsr	main			# main()
					# 1st proc 0 returns here
	clr.w	-(%sp)			# push format/offset word; frame type 0
	clr.l	-(%sp)			# set pc to 0
	mov.w	&MSP,-(%sp)		# set sr to user mode; set MSP bit
	rte				# execute user process

	data
romillv:
	long	Xtrap

	global	cputype
cputype:
	short	682
	text
#		Copyright (c) 1985 AT&T		#
#		All Rights Reserved		#
#	@(#)cswitch.m4	6.15	

# MC68020 version
# This file contains the routines used for context switching on UNIX V/68.
# The 1.1 level was derived from the 68000 cswitch.s level 1.3.

#	The routines contained here are:

	global	setjmp		# set up environment for external goto
	global	longjmp		# non-local goto
	global	save		# saves register sets
	global	resume		# restores register sets & u, then longjmps

	set	USIZE,4096	# number of bytes in the static u area
	set	HIGHPRI,0x3700	# supervisor state priority 7
	set	REGMASK,0xfefc	# save/restore a7-a1,d7-d2
	set	SP,48		# offset into savearea of SP


# setjmp (save_area)
# save (save_area)
#	saves registers and return location in save_area


	text
setjmp:
save:
	mov.l	(%sp)+,%a1	# pop return address of calling routine into %a1
	mov.l	(%sp),%a0	# address of save area
	movm.l	&REGMASK,(%a0)	# save a7-a1,d7-d2
	mov.l	&0,%d0		# return(0)
	jmp	(%a1)


# longjmp (save_area)
#	resets registers from save_area and returns to the location
#	from WHERE setjmp() WAS CALLED.
#	** Never returns 0 - this lets the returned-to environment know that the
#	return was the result of a long jump.

longjmp:
	mov.l	4(%sp),%a0	# get address of save_area
	movm.l	(%a0),&REGMASK	# restore a7-a1,d7-d2
	mov.l	&1,%d0		# return(1)
	jmp	(%a1)



# resume (&u, save_area)
#	switches to another process's "u" area. Returns non-zero.

# This section is unique to machines like the EXORmacs which have no
# mapping in the supervisor state.  The current process's "u" area has been
# saved by a call to the mmu driver.  This section copies the new "u"
# area into the global structure "u".  On other machines, this section would
# be replaced by the necessary code to map the new "u" area to the system
# "u" window

resume:

#
	tst.w	do_fpu		# Check for fpu on system.
	beq.b	no_fpu1

	mov.l	fpu_ptr,%a0
	btst	&0,3(%a0)	# CHECK FOR UFPRWRT
	bne.b	no_fpu1
#	fmovm.x	&0,(%a0)	#  Kludge until fsave bugs are fixed.

	fsave	124(%a0)	#  Save internal state.
	tst.b	124(%a0)	#  Check for null internal state.
	beq.b	no_fpu1

	fmovm.x	&allfpi,16(%a0)	# Save floating point registers.
	fmovm.l	%control,%status,%iaddr,112(%a0)	# and system regs.

no_fpu1:
	mov.l	8(%sp),%a0	# address of save_area
	mov.w	%sr,%d0		# save current PS
	mov.w	&HIGHPRI,%sr	# inhibit interrupts
	set	RP,0		# Root Pointer register
	set	MMUADDR,0x10000	# in CPU space

	mov.l	cacr,%a1	# Clear old cache entries.
	mov.l	%a1,%cacr

	mov.l	4(%sp),%a1	# address of new "u" areas PTEs
	mov.l	mmuupde,%a2	# address of global "u" areas PTEs

	mov.l	(%a1)+,(%a2)+	# get the users valid PTE
	mov.l	(%a1)+,(%a2)+	# ditto (4 descrs for 4K Uvec)
	mov.l	(%a1)+,(%a2)+	# ditto (4 descrs for 4K Uvec)
	mov.l	(%a1)+,(%a2)+	# ditto (4 descrs for 4K Uvec)

	tst.w	do_pmmu
	bne.b	pmmuflsh
# this is MMB code!
	mov.l	&RP+MMUADDR,%a2	# flush the ATC by re-writing root pointer
	mov.l	%dfc,%a1
	mov.l	&7,%d1
	mov.l	%d1,%dfc
	mov.l	fcltbl,%d1
	movs.l	%d1,(%a2)
	mov.l	%a1,%dfc
	bra.b	pastpmmu
# end MMB
pmmuflsh:
	mov.l	fcltbl,rpstore+4
	nop
	short	0xf039
	short	0x4c00
	long	rpstore

	nop
	short	0xf000		# PMMU flush all instruction.
	short	0x2400
				# at this point, the new process
				# u-vector will magically appear at
				# location 0x1000000 to 0x1000000+USIZE-1

pastpmmu:
	movm.l	(%a0),&REGMASK	# restore previous a7-a1,d7-d2
	mov.w	%d0,%sr		# restore priority

	tst.w	do_fpu		# Check for fpu chip.
	beq.b	no_fpu2

	mov.l	fpu_ptr,%a0
	tst.b	124(%a0)	# Was state null?
	beq.b	L%47

	fmovm.x	16(%a0),&allfpi	# Restore floating point regs.
	fmovm.l	112(%a0),%control,%status,%iaddr	# and system regs.
	frestore	124(%a0)
	and.l	&-2,(%a0)
	bra.b	L%49
L%47:
	frestore	124(%a0)	#Restore
	btst	&0,3(%a0)	# Check UFPRWRT flag.
	beq.b	L%49

	fmovm.x	16(%a0),&allfpi	# Restore floating point regs.
	fmovm.l	112(%a0),%control,%status,%iaddr	# and system regs.

	and.l	&-2,(%a0)
L%49:
no_fpu2:

	mov.l	&1,%d0		# return (1)
	jmp	(%a1)

#		Copyright (c) 1985 AT&T		#
#		All Rights Reserved		#
#	@(#)userio.m4	6.3	

# MC68020 version
	file	"userio.s"
#
#	errret: return here is address is bad
	text
	global	errret
errret:
	mov.l	&-1,%d0		# return -1 on bad address
	rts
#
#
#
	set	UD,1		# User-Data function code
	set	UI,2		# User-Instruction function code
	global	nofulow
nofulow:
#
#
#	fubyte(addr)
	global	fubyte
fubyte:
#
#	fuibyte(addr)
	global	fuibyte
fuibyte:
	mov.l	u+u_procp,%a0		# check if process is a server.
	tst.w	p_sysid(%a0)		# If so, handle it through rfubyte.
	beq.b	kfubyte
	jmp	rfubyte

kfubyte:
	mov.l	4(%sp),%a0		# get address
	mov.l	&0,%d0			# clear high order byte
	movs.b	(%a0),%d0		# fetch byte
	rts
#
#
#	fushort(addr)
	global	fushort
fushort:
	mov.l	4(%sp),%a0		# get address
	mov.l	&0,%d0			# clear high order byte
	movs.w	(%a0),%d0		# fetch byte
	rts
#
#	fuword(addr)
	global	fuword
fuword:
#
#	fuiword(addr)
	global	fuiword
fuiword:
	mov.l	u+u_procp,%a0		# check if process is a server.
	tst.w	p_sysid(%a0)		# If so, handle it through rfuword.
	beq.b	kfuword
	jmp	rfuword

kfuword:
	mov.l	4(%sp),%d0		# get address to test
	btst	&0,%d0			# address odd?
	bne	aderr			# opps
	mov.l	%d0,%a0			# set address
	movs.l	(%a0),%d0		# fetch word
	rts

#
#	subyte(addr,byte)
	global	subyte
subyte:
#
#	suibyte(addr,byte)
	global	suibyte
suibyte:
	mov.l	u+u_procp,%a0		# check if process is a server.
	tst.w	p_sysid(%a0)		# If so, handle it through rsubyte.
	beq.b	ksubyte
	jmp	rsubyte

ksubyte:
	mov.l	4(%sp),%a0		# get address
	mov.l	8(%sp),%d0		# get byte
	movs.b	%d0,(%a0)		# store byte
	mov.l	&0,%d0			# set no error
	rts
#
#	sushort(addr,byte)
	global	sushort
sushort:
	mov.l	4(%sp),%a0		# get address
	mov.l	8(%sp),%d0		# get byte
	movs.w	%d0,(%a0)		# store byte
	mov.l	&0,%d0			# set no error
	rts
#
#	suword(addr,word)
	global	suword
suword:
#
#	suiword(addr,word)
	global	suiword
suiword:
	mov.l	u+u_procp,%a0		# check if process is a server.
	tst.w	p_sysid(%a0)		# If so, handle it through rsuword.
	beq.b	ksuword
	jmp	rsuword

ksuword:
	mov.l	4(%sp),%d0		# get address to test
	btst	&0,%d0			# address odd?
	bne.b	aderr			# opps
	mov.l	%d0,%a0			# set address
	mov.l	8(%sp),%d0		# get byte
	movs.l	%d0,(%a0)		# store byte
	mov.l	&0,%d0			# set no error
	rts
aderr:
	mov.l	&-1,%d0			# had misalignment, set error
	rts

#
# upath(from, to, maxbufsize);
#  Copies a path from user space to kernel space.
#

	global	upath
upath:
	mov.l	u+u_procp,%a0		# check if process is a server.
	tst.w	p_sysid(%a0)		# If so, handle it through spath.
	bne	spath

	mov.l	4(%sp),%a0		# Pick up from address.
	mov.l	%a0,%a1

	mov.l	&-1,%d0			# Calculate the length of
ulnloop:
	add.l	&1,%d0			# the string.
	movs.b	(%a1)+,%d1
	tst.b	%d1
	bne.b	ulnloop

	cmp.l	%d0,12(%sp)		# If its too big, go to
	bhs	plenerr			# error return.

	mov.l	8(%sp),%a1		# Pick up to address.
umvloop:
	movs.b	(%a0)+,%d1		# Move it.
	mov.b	%d1,(%a1)+
	bne.b	umvloop

	rts				# Return path length in d0.

# uzero(vaddr, size)
#	addr is a user virtual address
#	size is in bytes

	global	uzero
uzero:	mov.l	4(%sp),%a0	# address
	mov.l	8(%sp),%d1	# number of bytes
	ble	L%done

	sub.l	%a1,%a1		# constant zero
	mov.w	%a0,%d0		# long align address
	and.w	&3,%d0
	beq.b	L%uzl1		# already aligned

	neg.w	%d0		# compute (4-addr%4)-1 (i.e. loop count)
	add.w	&3,%d0
L%uzl6:	movs.b	%a1,(%a0)+		# clear one byte
	sub.l	&1,%d1		# decrement count by one byte
	dbeq	%d0,L%uzl6	# loop while not aligned & more bytes

L%uzl1:	mov.l	%d1,%d0		# save count
	lsr.l	&2,%d1		# adjust count for long loop
	bra.b	L%uzl5

L%uzl2:	swap	%d1		# outer dbra loop
L%uzl3:	movs.l	%a1,(%a0)+	# inner dbra loop: clear longs
L%uzl5:	dbra	%d1,L%uzl3
	swap	%d1
	dbra	%d1,L%uzl2

	and.w	&3,%d0
	bra.b	L%uzb1
L%uzb2:	movs.b	%a1,(%a0)+
L%uzb1:	dbra	%d0,L%uzb2
	bra	L%done


	global	copyin
	global 	copyout
	global	short_copyin
	global 	short_copyout
copyin:
#
#  copyin copies data from the user address space into the kernel 
# address space.  The calling sequence is:
#
# 	copyin(useraddr,kernaddr,count);
#
	mov.l	u+u_procp,%a0		# check if process is a server.
	tst.w	p_sysid(%a0)		# If so, handle it through rcopyin
	beq.b	kcopyin
	jmp	rcopyin

kcopyin:	
	bsr.w	getcpyp		#  Call getcpyp to get all of the 
	beq.b	cinret		# passed parameters into registers and
#				  if the count is <= 0, exit immediately.
#
#  If possible, align the start and end addresses on a long word
#  boundary for efficiency sake.
#
	cmp.w	%d1,&3		#  If the byte count is less than 3,
	bls.b	cinbyte		# the following tests won't work so
#				  go move bytes instead.
	mov.w	%a0,%d0		# Get the least significant bit of 
	lsr.w	&1,%d0		# the address to see if we are on
	bcc.b	cineven		# an odd boundary. If not, don't move.
#
	movs.b	(%a0)+,%d0	# move one byte to get to even address
	mov.b	%d0,(%a1)+
	sub.l	&1,%d1		# decrement count by one byte
cineven:
	lsr.w	&1,%d0		# Check for alignment on a long word
	bcc.b	cinlong		# boundary.  If its not, then move
	movs.w	(%a0)+,%d0	# move one word to get to long address
	mov.w	%d0,(%a1)+
	sub.l	&2,%d1		# long word and decrement byte count by 2.
#
#
#  Move all of the longs that we can.  a0 (and possibly a1) is on a long
# word boundary.
#
cinlong: mov.l	%d1,%d0		# Put count in d0 for finding
	and.l	&3,%d1		# number of longs and put remainder in d1.
	mov.l	%d1,12(%sp)	# Save the remainder for later use.
	lsr.l	&2,%d0		# Adjust count to the number of longs
	beq.b	cinrem		# and if zero, go do the remainder.

cinnxt:
	movs.l	(%a0)+,%d1	# Move the next long.
	mov.l	%d1,(%a1)+
	sub.l	&1,%d0		# decrement long count and if
	bne.b	cinnxt		# If count is positive, continue.
#
#  Move any remaining bytes.
#
	
	mov.l	12(%sp),%d1	# Restore the remainder count to d1.
cinrem:
	tst.l	%d1		#  Are there any remaining bytes
	beq.b	cinret		# to move? If not, return.
#
cinbyte:
	movs.b	(%a0)+,%d0	# Move the last bytes.  d1 has
	mov.b	%d0,(%a1)+
	sub.l	&1,%d1		# the remaining byte count.
	bne.b	cinbyte
#
cinret:
	mov.l	&0,%d0		# Return 0.
	rts

#
# short_copyin	- copyin that won't use 32 bit buss xfers
#
short_copyin:	
	bsr.b	getcpyp		#  Call getcpyp to get all of the 
	beq.b	cinret		# passed parameters into registers and
#				  if the count is <= 0, exit immediately.
#
#  If possible, align the start and end addresses on a long word
#  boundary for efficiency sake.
#
	cmp.w	%d1,&3		#  If the byte count is less than 3,
	bls.b	cinbyte		# the following tests won't work so
#				  go move bytes instead.
	mov.w	%a0,%d0		# Get the least significant bit of 
	lsr.w	&1,%d0		# the address to see if we are on
	bcc.b	cinshort	# an odd boundary. If not, don't move.
#
	movs.b	(%a0)+,%d0	# move one byte to get to even address
	mov.b	%d0,(%a1)+
	sub.l	&1,%d1		# decrement count by one byte
#
#
#  Move all of the shorts that we can.  a0 (and possibly a1) is on a
# word boundary.
#
cinshort:
	mov.l	%d1,%d0		# Put count in d0 for finding
	and.l	&1,%d1		# number of shorts and put remainder in d1.
	mov.l	%d1,12(%sp)	# Save the remainder for later use.
	lsr.l	&1,%d0		# Adjust count to the number of shorts.
	beq.b	cinrem		# and if zero, go do the remainder.

cinnxt_short:
	movs.w	(%a0)+,%d1	# Move the next short.
	mov.w	%d1,(%a1)+
	sub.l	&1,%d0		# decrement short count and if
	bne.b	cinnxt_short	# If count is positive, continue.
#
#  Move any remaining bytes.
#
	
	mov.l	12(%sp),%d1	# Restore the remainder count to d1.
	bra.w	cinrem		# copy any remainder

#
#
#  Local subroutine 
#
getcpyp:
	mov.l	4+4(%sp),%a0	#  Get the source address into a0
	mov.l	4+8(%sp),%a1	# and destination into a1.
	mov.l	4+12(%sp),%d1	#  Get the count into d1 and set
	rts			# condition codes and return.

copyout:
#
#  copyout copies data from the kernel address space into the user 
# address space.  The calling sequence is:
#
# 	copyout(kernaddr,useraddr,count);
#

	mov.l	u+u_procp,%a0		# check if process is a server.
	tst.w	p_sysid(%a0)		# If so, handle it through rcopyout.
	beq.b	kcopyout
	jmp	rcopyout

kcopyout:
	bsr.b	getcpyp		#  Call getcpyp to get all of the 
	beq.b	cout		# passed parameters into registers and
#				  if the count is <= 0, exit immediately.
#
#  If possible, align the start and end addresses on a long word
#  boundary for efficiency sake.
#
	cmp.w	%d1,&3		#  If the byte count is less than 3,
	bls.b	coutbyte	# the following tests won't work so
#				  go move bytes instead.
	mov.w	%a0,%d0		# Get the least significant bit of 
	lsr.w	&1,%d0		# the address to see if we are on
	bcc.b	couteven	# an odd boundary. If not, don't move.
#
	mov.b	(%a0)+,%d0	# move one byte to get to even address
	movs.b	%d0,(%a1)+
	sub.l	&1,%d1		# decrement count by one byte
couteven:
	lsr.w	&1,%d0		# Check for alignment on a long word
	bcc.b	coutlong	# boundary.  If its not, then move
	mov.w	(%a0)+,%d0	# move one word to get to long address
	movs.w	%d0,(%a1)+
	sub.l	&2,%d1		# long word and decrement byte count by 2.
#
#
#  Move all of the longs that we can.  a0 (and possibly a1) is on a long
# word boundary.
#
coutlong: mov.l	%d1,%d0		# Put count in d0 for finding
	and.l	&3,%d1		# number of longs and put remainder in d1.
	mov.l	%d1,12(%sp)	# Save the remainder for later use.
	lsr.l	&2,%d0		# Adjust count to the number of longs
	beq.b	coutrem		# and if zero, go do the remainder.

outnxt:
	mov.l	(%a0)+,%d1	# Move the next long.
	movs.l	%d1,(%a1)+
	sub.l	&1,%d0		# decrement long count and if
	bne.b	outnxt		# If count is positive, continue.
#
#  Move any remaining bytes.
#
	
coutrem:
	mov.l	12(%sp),%d1	#  Restore the remainder count to d1.
	tst.l	%d1		#  Are there any remaining bytes
	beq.b	cout		# to move? If not, return.
#
coutbyte:
	mov.b	(%a0)+,%d0	# Move the last bytes.  d1 has
	movs.b	%d0,(%a1)+
	sub.l	&1,%d1		# the remaining byte count.
	bne.b	coutbyte
#
cout:	mov.l	&0,%d0		# return (0)
	rts

short_copyout:
	bsr.b	getcpyp		#  Call getcpyp to get all of the 
	beq.b	cout		# passed parameters into registers and
#				  if the count is <= 0, exit immediately.
#
#  If possible, align the start and end addresses on a long word
#  boundary for efficiency sake.
#
	cmp.w	%d1,&3		#  If the byte count is less than 3,
	bls.b	coutbyte	# the following tests won't work so
#				  go move bytes instead.
	mov.w	%a0,%d0		# Get the least significant bit of 
	lsr.w	&1,%d0		# the address to see if we are on
	bcc.b	coutshort	# an odd boundary. If not, don't move.
#
	mov.b	(%a0)+,%d0	# move one byte to get to even address
	movs.b	%d0,(%a1)+
	sub.l	&1,%d1		# decrement count by one byte
coutshort:
#
#
#  Move all of the longs that we can.  a0 (and possibly a1) is on a long
# word boundary.
#
	mov.l	%d1,%d0		# Put count in d0 for finding
	and.l	&1,%d1		# number of longs and put remainder in d1.
	mov.l	%d1,12(%sp)	# Save the remainder for later use.
	lsr.l	&1,%d0		# Adjust count to the number of longs
	beq.b	coutrem		# and if zero, go do the remainder.

outnxtshort:
	mov.w	(%a0)+,%d1	# Move the next long.
	movs.w	%d1,(%a1)+
	sub.l	&1,%d0		# decrement long count and if
	bne.b	outnxtshort	# If count is positive, continue.
#
#  Move any remaining bytes.
#
	bra.w	coutrem

	global	nofuhigh
nofuhigh:
#		Copyright (c) 1985 AT&T		#
#		All Rights Reserved		#
#	@(#)math.m4	6.1 	

# MC68020 version

	text

# unsigned min(a,b)
# unsigned a,b;
# {	return a <= b ? a : b; }

	global	min
min:
	mov.l	4(%sp),%d0
	cmp.l	%d0,8(%sp)
	bls.b	L%min
	mov.l	8(%sp),%d0
L%min:
	rts

# unsigned max(a,b)
# unsigned a,b;
# {	return a >= b ? a : b; }

	global	max
max:
	mov.l	4(%sp),%d0
	cmp.l	%d0,8(%sp)
	bhs.b	L%max
	mov.l	8(%sp),%d0
L%max:
	rts

# log2(value)
# returns log2 value of passed parameter.
# Note:  Assumes only 1 bit set in passed parameter.  Bad results otherwise.

	global	log2
log2:
	mov.l	4(%sp),%d0		# Get number to be logged.
	bfffo	%d0{&0:&32},%d1		# Find offset to first 1.
	mov.l	&0x1f,%d0		# (From the wrong end)
	sub.l	%d1,%d0
	rts
#		Copyright (c) 1985 AT&T		#
#		All Rights Reserved		#
#	@(#)misc.s	1.0	

	text

spl0:	global	spl0
	mov.w	%sr,%d0		# return current ps value
	and.w	&0x3000,%sr	# set new priority level, Supervisor and master
	rts

spl1:	global	spl1
	mov.w	%sr,%d0		# return current ps value
	or.w	&0x0100,%sr
	and.w	&0x3100,%sr	# set new priority level, S and M bits
	rts

spl2:	global	spl2
	mov.w	%sr,%d0		# return current ps value
	or.w	&0x0200,%sr
	and.w	&0x3200,%sr	# set new priority level, S and M bits
	rts

spl3:	global	spl3
	mov.w	%sr,%d0		# return current ps value
	or.w	&0x0300,%sr
	and.w	&0x3300,%sr	# set new priority level, S and M bits
	rts

spl4:	global	spl4
	mov.w	%sr,%d0		# return current ps value
	or.w	&0x0400,%sr
	and.w	&0x3400,%sr	# set new priority level, S and M bits
	rts

spltty:	global	spltty
spl5:	global	spl5
	mov.w	%sr,%d0		# return current ps value
	or.w	&0x0500,%sr
	and.w	&0x3500,%sr	# set new priority level, S and M bits
	rts

spl6:	global	spl6
	mov.w	%sr,%d0		# return current ps value
	or.w	&0x0600,%sr
	and.w	&0x3600,%sr	# set new priority level, S and M bits
	rts

splhi:	global	splhi
spl7:	global	spl7
	mov.w	%sr,%d0		# return current ps value
	or.w	&0x0700,%sr	# set new priority level, S and M bits
	rts

splx:	global	splx		# set ipl to input argument value
	mov.w	%sr,%d0		# get current sr value
	or.w	&0x0700,%d0	# set all bits in IPL field
	mov.w	6(%sp),%d1	# get new sr from argument
	or.w	&0x3000,%d1	# insure that S and M bits don't change
	and.w	%d0,%d1		# create new sr with old S and M bits
	mov.w	%sr,%d0		# return current (old) sr value
	mov.w	%d1,%sr		# set new priority level
	rts

	set	OKINTS,0x3000	# enable interrupts
# idle()

	global	waitloc
	global	idle
idle:
	stop	&OKINTS		# stop & wait for an interrupt
waitloc:
	rts

	set	NS,2	# shift to get nbr of long moves/clears

# bcopy(from, to, bytes)
#	"to" and "from" are both system physical addresses

	global	bcopy
bcopy:	mov.l	4(%sp),%a0	# from
	mov.l	8(%sp),%a1	# to
	mov.l	12(%sp),%d1	# number of bytes
	ble.b	L%done
	mov.w	%a0,%d0		# long align (at least) one address
	and.w	&3,%d0
	beq.b	L%cl1		# already aligned
	neg.w	%d0		# compute (4-addr%4)-1 (i.e. loop count)
	add.w	&3,%d0
L%cl7:	mov.b	(%a0)+,(%a1)+	# move one byte
	sub.l	&1,%d1		# decrement count by one byte
	dbeq	%d0,L%cl7	# loop while not aligned & more bytes
L%cl1:	mov.l	%d1,%d0		# save count
	lsr.l	&NS,%d1		# adjust count for long loop
	bra.b	L%cl5

L%cl2:	swap	%d1		# outer dbra loop
L%cl3:	mov.l	(%a0)+,(%a1)+	# inner dbra loop: move longs
L%cl5:	dbra	%d1,L%cl3
	swap	%d1
	dbra	%d1,L%cl2

	and.w	&3,%d0
	bra.b	L%cl6
L%cl9:	mov.b	(%a0)+,(%a1)+
L%cl6:	dbra	%d0,L%cl9

L%done:	mov.l	&0,%d0		# return (0)
	rts

# bzero(addr, size)
#	addr is a physical address
#	size is in bytes

	global	bzero
bzero:	mov.l	4(%sp),%a0	# address
	mov.l	8(%sp),%d1	# number of bytes
	ble.b	L%done
	sub.l	%a1,%a1		# constant zero
	mov.w	%a0,%d0		# long align address
	and.w	&3,%d0
	beq.b	L%zl1		# already aligned
	neg.w	%d0		# compute (4-addr%4)-1 (i.e. loop count)
	add.w	&3,%d0
L%zl6:	clr.b	(%a0)+		# clear one byte
	sub.l	&1,%d1		# decrement count by one byte
	dbeq	%d0,L%zl6	# loop while not aligned & more bytes
L%zl1:	mov.l	%d1,%d0		# save count
	lsr.l	&NS,%d1		# adjust count for long loop
	bra.b	L%zl5

L%zl2:	swap	%d1		# outer dbra loop
L%zl3:	mov.l	%a1,(%a0)+	# inner dbra loop: clear longs
L%zl5:	dbra	%d1,L%zl3
	swap	%d1
	dbra	%d1,L%zl2

	and.w	&3,%d0
	bra.b	L%zb1
L%zb2:	clr.b	(%a0)+
L%zb1:	dbra	%d0,L%zb2
	bra	L%done


# caddr_t bsmear( addr, nbytes )

# set nbytes of ram to a stable state, starting at addr,
# until a bus error occurs, or nbytes are done.
# nbytes should be a multiple of INCR.

# return the ending address: either the address on which a fault occured
# (or slightly more than that), or addr + nbytes.

# some rams can come up with parity errors.
# each byte must be written before it # is read.
# a tst or clr instruction can fail even though the memory exists.
# hence, this routine actually writes all bytes until an error is met.

	set	NOINTS,0x0700	# sr mask: ipl 7
	set	INCR,4		# number of bytes cleared at a time
	set	L2INCR,2	# log base 2 of INCR
#define(rmstkfrm,
#`	mov.w	6(%sp),%d0	# get the stack format
	#lsr.w	&12,%d0		# shift to low order byte
	#mov.b	(stkfmtsz,%d0.w),%d0	# look up size of the saved stack
	#add.w	&8,%d0		#  adjust to full saved stack size
	#add.w	%d0,%sp		# adjust stack to remove
#')

bsmear:	global	bsmear
	mov.l	4(%sp),%a0	# addr
	mov.l	8(%sp),%d0	# nbytes
	lsr.l	&L2INCR,%d0	# nbytes / INCR
	mov.w	&INCR,%a1	# constant INCR
	mov.w	%sr,%d1 	# save status register
	or.w	&NOINTS,%sr	# block interrupts
	mov.l	(8,[vbr]),-(%sp)	# save the current bus handler
	mov.l	&L%bs9, (8,[vbr])	# catch bus error exceptions
	bra.b	L%bs3		# go start the loop

L%bs1:	swap	%d0		# outer dbra loop
L%bs2:	clr.l	(%a0)		# inner dbra loop: smear INCR bytes
	add.l	%a1,%a0
L%bs3:	dbra	%d0,L%bs2
	swap	%d0
	dbra	%d0,L%bs1

L%bs4:	mov.l	(%sp)+,(8,[vbr])	# restore bus error handler
	mov.w	%d1,%sr		# restore status register
	mov.l	%a0,%d0		# return pointer in both registers
	rts

# L%bs9 can be reached only by a bus error exception trap

L%bs9:	#rmstkfrm		# lookup how much to throw away
	mov.w	6(%sp),%d0	# get the stack format
	lsr.w	&12,%d0		# shift to low order byte
	mov.b	(stkfmtsz,%d0.w),%d0	# look up size of the saved stack
	add.w	&8,%d0		#  adjust to full saved stack size
	add.w	%d0,%sp		# adjust stack to remove
	mov.l	&0xfffb0032,%a1	# Address of STAT2 on 131 card
	mov.b	(%a1),%d0	# Read to clear BERR source status register
	bra.b	L%bs4



# int bprobe( addr, byte )
# caddr_t addr;
# int byte;

# if "byte" < 0, read the byte at "addr" to see if a bus error occurs.
# else write (byte) "byte" into address "addr" and see if a bus error occurs.

# return true (1) if the error occurs, else false (0).

bprobe:	global	bprobe
	mov.w	%sr,%d1 	# save status register
	or.w	&NOINTS,%sr	# block interrupts

	mov.l	%vbr,%a0	# Save current bus error handler.
	mov.l	8(%a0),%a1
	mov.l	&L%bpr9,8(%a0)	# catch bus error exceptions


	mov.l	4(%sp),%a0	# "addr"
	mov.l	8(%sp),%d0	# get the int "byte"
	bge.b	L%bpr1		# skip for write
	tst.b	(%a0)		# read (byte) "byte" at "addr"
	bra.b	L%bpr2
L%bpr1:	mov.b	%d0,(%a0)	# write (byte) "byte" into "addr"
L%bpr2:	mov.l	&0,%d0		# false return value
L%bpr3:	
	mov.l	%vbr,%a0	# Restore bus error handler.
	mov.l	%a1,8(%a0)
	mov.w	%d1,%sr		# restore status register
	rts

# L%bpr9 can be reached only by a bus error exception trap

L%bpr9:	#rmstkfrm		# true return value
	mov.w	6(%sp),%d0	# get the stack format
	lsr.w	&12,%d0		# shift to low order byte
	mov.b	(stkfmtsz,%d0.w),%d0	# look up size of the saved stack
	add.w	&8,%d0		#  adjust to full saved stack size
	add.w	%d0,%sp		# adjust stack to remove
	mov.l	&0xfffb0032,%a0	# Address of STAT2 on 131 card
	mov.b	(%a0),%d0	# Read to clear BERR source status register
	mov.l	&1,%d0		# true return value
	bra.b	L%bpr3


	global	icacheon, icacheoff

icacheon:
#  Flush (may have stale data) and enable the internal chip cache
	mov.l	&CACHENB,%d0
	mov.l	%d0,cacr
	mov.l	%d0,%cacr
	rts

icacheoff:
#  Disable the internal chip cache
	mov.l	&CACHDIS,%d0
	mov.l	%d0,cacr
	mov.l	%d0,%cacr
	rts

	data
cacr:	long	CACHENB

	text
	global	breakpoint
breakpoint:
	illegal
	rts

########################################################################
#	searchdir(buf, n, target) - search a directory for target
#	returns offset of match, or empty slot, or -1
#
#	This code could be (and previously was) written in C.  Release
#	5.2.2 has this routine written in assembly for the sake of speed.
#
#       Register usage:
#	%d0 is used to contain the count of the remaining size of the 
#          directory as we search through it.
#	%a0 and %a2 are used to index into the directory as we search it for 
#          the name or empty slot. %a0 is the temporary used during the
#          name comparisons and %a2 always points to the base of the
#	   current entry being compared.
#       %a1 and %d2 are used to hold the address of the name we are 
#          searching for. %a1 is the temporary copy during the actual
#          comparison and %d2 holds a permanent copy.  Having d2 hold
#          the permanent is faster than retrieving it from the stack
#          each time its needed.
#       %d3 is used to hold the address of the first empty slot found
#          or 0 if no emptys have been found.
#	%d4 is used to contain the size of a directory entry which is
#	   16 bytes.
#
#	%d1 is scratch.
########################################################################

	global	searchdir

	text
searchdir:
	movm.l	&0x3820,-(%sp)			# Save registers.
	
	mov.l	4+16(%sp),%a0			#  Get directory pointer
	mov.l	%a0,%a2				#  Save a copy in a2
	mov.l	8+16(%sp),%d0			#  Get size of directory.
	lsr.l	&4,%d0				#  Get number of entries.
	beq.b	sdone				#  No entries?
	sub.l	&1,%d0

	mov.l	12+16(%sp),%d2			#  Get name pointer in d2.
	mov.l	&16,%d4				#  Sizeof direct.
	mov.l	&0,%d3				#  No empty slots have been
						#  found yet.

srchloop:
	tst.w	(%a0)+				# Check for valid entry.
	beq.b	sempty				# branch on emtpy slot.

	mov.l	&7-1,%d1			# Put sizeof name in d1
	mov.l	%d2,%a1				# Get pointer to name in a1.

#
#  Note that the following cmpnames loop has made the assumption that a0 and
#  a1 point at entries that are 0 filled beyond the end of the names in the
#  entries.  A search of the code that calls searchdir indicated that this
#  would always be the case, but if changes in the future are made that
#  rule that assumption out, then cmpnames will have to change.
#
cmpnames:
	cmp.w	(%a0)+,(%a1)+			# Compare the names.  Generally,
	dbne	%d1,cmpnames			# this loop exits at the first
						# or second character and thus
						# is fast for the normal case.
	beq.b	fndname				# If we have a match, branch.

						# No match.
scont:
	add.l	%d4,%a2				# Increment pointer and 
	mov.l	%a2,%a0				# decrement remaining byte
#	sub.l	%d4,%d0				# count.

#srchchk:
#	cmp.l	%d0,%d4			#  If less than 16 (direct size) bytes	
#	bhs.b	srchloop		# remain, wer'e done.  Else continue
					# searching.
	dbra	%d0,srchloop

sdone:
#
#  Control comes here when we have searched the entire directory and
# have not found a match.  We must return the offset of the empty slot
# that we have found or -1 if we haven't found any emptys.
#
	mov.l	&-1,%d0			#  Return -1 if no empty
	tst.l	%d3			# slots were found.
	beq.b	sfail

	sub.l	4+16(%sp),%d3		#  Otherwise calculate the
	mov.l	%d3,%d0			# offset of the empty slot and
					# return it instead.
sfail:
	movm.l	(%sp)+,&0x41c
	rts

sempty:
#
#  Control comes here when we find any empty slot during the search of
#  the directory.
#
	tst.l	%d3			# Have we already found an
	bne.b	scont			# empty slot?  If so, don't
					# update it.
	mov.l	%a0,%d3			# This is the first empty we
	sub.l	&2,%d3			# have found, so save in d3.
	bra.b	scont			# and continue searching.
	
	

fndname:
#
#  We found a match if we come here.
#
	sub.l	4+16(%sp),%a2			# Convert to offset.
	mov.l	%a2,%d0				# Return the offset.
	movm.l	(%sp)+,&0x41c
	rts

#
#  spath(from, to, maxbufsize);
#  Copies a pathname from kernel to kernel space.
#  Returns length of path or -2 to indicate path was too long to
# fit.
#

	global	spath
spath:
	mov.l	4(%sp),%a0		# Pick up from address.
	mov.l	%a0,%a1

	mov.l	&-1,%d0			# Calculate the length of
slnloop:
	add.l	&1,%d0			# the string.
	tst.b	(%a1)+
	bne.b	slnloop

	cmp.l	%d0,12(%sp)		# If its too big, go to
	bhs.b	plenerr			# error return.

	mov.l	8(%sp),%a1		# Pick up to address.
smvloop:
	mov.b	(%a0)+,(%a1)+		# Move it.
	bne.b	smvloop

	rts				# Return path length in d0.

plenerr:
	mov.l	&-2,%d0
	rts

	global	rcopyfault
rcopyfault:
	clr.l	u+u_caddrflt
	mov.l	&-1,%d0
	rts

# This module handles the trace mode exceptions to allow kernel tracing w/o
# affecting sdb type tracing (including trap 0'S). It is going to get
# the BUG trace handler address from location 0x24 - the BUG's vector
# table. We have been assured that there will be a BUG vector table
# at location 0.
#
#
#	Exception type:		Action:
#
#	user mode		go to trap()
#	trace of user trap	ignore the trace exception (trap will handle)
#	supervisor		go to monitor BUG trace handler
#

	global	Xtrap

trceintr:	global trceintr

	btst	&5,(%sp)		# test supervisor bit of saved %sr
	beq.b	L%user			# from user mode, go to trap()
	cmp.l	2(%sp),&nullvect	# see if the PC is at nullvect
	beq.b	L%call			# traced user trap, finish trap handler
	cmp.l	2(%sp),&Xsyscall	# see if the PC is at Xsyscall
	beq.b	L%call			# traced user syscall, finish syscall

# now we know that this was a traced supervisor instr, so go to BUG

	jmp	([0x24])		# Jump through bug vectors.
L%user:	jmp	Xtrap			# go to trap
L%call: and.w	&0x3fff,(%sp)		# clear trace bits in sr
	rte				# and return

#
#
#	This module handles the illegal instruction interrupt.
#	It too will get the BUG routine address from the BUG's vector
#  	table at location 0.
#
#
instintr:	global instintr
	btst	&5,(%sp)		# test suppervisor bit of saved sr
	bne.b	L%bug
	jmp	Xtrap			# go to trap

L%bug:	
	jmp	([0x10])		# Jump through bug vectors.
#
#
#
#	This module handles getting to the BUG's Software Abort routine
#
#
swabintr:	global swabintr
	jmp	([0x100])		# Jump through bug vector.
#
#
#
dump%:	global	dump%
	bra	dump%

#
#	The following routine (chk_fpu) sets the global "do_fpu" to indicate 
#	whether or not a working 881 was found.
#	It also returns the 881's internal reset state and the null
#	881 programmer's model, when possible.
#
#	void chk_fpu(fsave,fpregs)
#	char   fsave[4];
#	fregs  *fpregs;
#	{}
#

	data
reset:	short 0				# a null (reset) frame for the M68881
	short 0
	short 0

        text
chk_fpu: global chk_fpu

	mov.w	&1,do_fpu		# flag fpu found to start with
	mov.l	%vbr,%a0
	mov.l	0x2c(%a0),-(%sp)	# save prev. f-line exception handler
	mov.l	&grabtrap,0x2c(%a0)
	frestore reset			# restore 881 null reset state
	fsave	([8,%sp])		# save 881 null reset state

contnu:	mov.l	(%sp)+,0x2c(%a0)	# restore prev. f-line handler

	tst.w	do_fpu
	beq.b	bye			# no fpu found, so exit


	tst.b	([4,%sp])		# Did we save the null state?
	beq.b	stateok

	clr.w	do_fpu			# Turn off fpu proc. - internal error
					# panic("881 fpu not in reset state");
	mov.l	&L%23,(%sp)
#	jsr	panic
	bra.b	bye

stateok:
	mov.l	8(%sp),%a0
	fmovm.x	&allfpi,(%a0)		# save all the fpi regs
	fmovm.l	%control,%status,%iaddr,96(%a0)
bye:	rts

grabtrap: clr.w do_fpu			# no 881 found, f-line trap occurred
	mov.l	&contnu,2(%sp)		# change return address
	rte				# return from exception

	data	2
L%23:
	byte	'8,'8,'1,0x20,'f,'p,'u,0x20
	byte	'n,'o,'t,0x20,'i,'n,0x20,'r
	byte	'e,'s,'e,'t,0x20,'s,'t,'a
	byte	't,'e,0x00
	text


#
#	void fpu_UN_clear() # Clear the UNordered (or NaN) in the 881s
#			    # %status registers so BSUN exceptions dont
#			    # happen over and over.
	global fpu_UN_c	    # <name truncated to 8 chars>
fpu_UN_c: 
	fmovm.l	%status,%d0
	bfclr	%d0{&7:&1}		# clear the NaN condition code bit
	fmovm.l	%d0,%status
	rts




#
#	void fsave(array)	# save the 881s internal state away
#	short *array;
#
fsave:
	global	fsave
	fmovm.x	&0,(4,%sp)	# kludge until 881 fsave bugs are fixed
	fsave	([4,%sp])
	rts




#
#	void frestore(array)	# restore the 881s internal state
#	short *array;
#
frestore:
	global	frestore
	frestore ([4,%sp])
	rts
#		Copyright (c) 1985 AT&T		#
#		All Rights Reserved		#
#	@(#)end.m4	6.3	

# MC68020 version
# This	is the code executed by proc(1) to load /etc/init.

	data
	global	icode		# location of boot code
	global	szicode		# size of icode in bytes

# Bootstrap program executed in user mode
# to bring up the system.

	set	exec%,11

icode:	mov.l	&I_stack,%sp
	mov.l	&exec%,%d0
	trap	&0			# exec( "/etc/init" );
L%here:	bra.b	L%here

L%arg:	long	I_ifile
	long	0
L%ifile:
	byte	'/,'e,'t,'c,'/,'i,'n,'i,'t,0
	even
L%stack:
	long	0
	long	I_ifile
	long	L%arg-icode
szicode:
	long	szicode-icode		# size of init code

	set	I_stack,L%stack-icode
	set	I_ifile,L%ifile-icode
