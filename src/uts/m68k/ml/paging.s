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
