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
