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
