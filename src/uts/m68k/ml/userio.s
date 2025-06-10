
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
