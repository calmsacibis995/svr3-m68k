# "@(#)dbdiv.s	6.1	";
	file	"dbdiv.s"

#+
#
#	M68000 IEEE Format Double Precision Routines
#
#	(C) Copyright 1983 by Motorola Inc.
#
#	Written by: Jim Hanko and Eric Randau
#
#
#  dbdiv is callable from 68k C
#  it supports ieee double precision floating point.
#  unnormalized numbers cause an underflow... underflow gives
#  no warning. this includes those underflows that cause a divide by
#  zero.... divide by zero returns + infinity
#  overflow returns +infinity.
#
#-
#        offsets for stack save area
	set	quotient,-10	# ds.w	5	quotient will end up here
	set	u,quotient-18	# ds.w	9	u is the dividend
	set	v,u-8		# ds.w	4	v is the divisor
	set	zero,v-2	# ds.w	1	v(0) = zero (must preceed v)
	set	sexpq,zero-2	# ds.w	1	sign + exponent of quotient
	set	size,sexpq	
 
# register masks

	set	nreg,-7*4
	set	reglist,0x04fc	#%a2,%d2-%d7

         text
	global	dbdiv%%	
 
dbdiv%%:	
#
#        on exit,   <%d0,%d1> = <%d0,%d1> / <4(%sp),8<%sp>
#                  quotient = dividend / divisor
#                     q     =    u     /   v
#
#       note:   this routine does not return unnormalized numbers
#               they are treated as zero.
#
#        this routine is based on knuth's "algorithm d" found in
#        volume 2 (seminumerical algorithms) second edition page 257
#
	link	%fp,&(nreg+size)
	movm.l	&reglist,(%sp)	# push nonvolatile regs
	mov.l  8(%fp),%d2
	mov.l	12(%fp),%d3

# compute the sign of the quotient
	mov.l	%d0,%d4	# copy sign + exp + part of mantissa (u)
	mov.l	%d2,%d5	# copy sign + exp + part of mantissa (v)
	eor.l	%d4,%d5	# see if signs are the same
	swap	%d5	
	lsr.w	&4,%d5	
	and.w	&0x800,%d5	# only the sign is left
	mov.w	%d5,sexpq(%fp)	# save the sign of the quotient
# compute exponent of the quotient
	mov.l	%d2,%d4	# extract the exp of v
	swap	%d4	
	lsr.w	&4,%d4	# right justified sign + exp
	and.w	&0x7ff,%d4	# remov sign
	beq	L%zdiv	# unnormalized, treat as 0. (divide by 0 error)
	cmp.w	%d4,&0x7ff	# check for nan or infinity.
	beq	L%naninf	# error
# here iff divisor (v) was not nan, unnormalized or infinity.
	mov.l	%d0,%d5	# extract exp of u
	swap	%d5	
	lsr.w	&4,%d5	# right justified sign + exp
	and.w	&0x7ff,%d5	# remov sign
	beq	L%iszero	# unnormalized, treat as zero(dividend=0,quotient=
	cmp.w	%d5,&0x7ff	# check for nan or infinity.
	beq	L%naninf	# error
	sub.w	%d4,%d5	# %d5 = exp of quotient
	add.w	&0x3ff,%d5	# adjust for excess 1023 arithmetic
	ble	L%iszero	# exp. is too small, answer is zer0
	cmp.w	%d5,&0x7ff	# check for result overflow
	bge	L%naninf	# return infinity
	or.w	%d5,sexpq(%fp)	# save exponent of quotient
	bset	&20,%d0	# real mantissa
	bset	&20,%d2	# real mantissa
#        step "%d5" of knuth's algorithm
	ror.l	&5,%d0	# normalize (per knuth) %d0,%d1   (shift left 11 bits)
	swap	%d0	
	ror.l	&5,%d1	# start of mov bits from %d1 to %d0
	swap	%d1	
	mov.w	%d1,%d5	# copy shifted bits
	and.w	&0x7ff,%d5	# mask for bits shifted
	and.w	&0xf800,%d0	# remov sign+exponent bits
	or.w	%d5,%d0	# put shifted bits in %d0
	eor.w	%d5,%d1	# remov shifted bits from %d1
	ror.l	&5,%d2	# normalize (per knuth) %d2,%d3 (shift left 11 bits)
	swap	%d2	
	ror.l	&5,%d3	# start of mov bits from %d3 to %d2
	swap	%d3	
	mov.w	%d3,%d5	# copy shifted bits
	and.w	&0x7ff,%d5	# mask for bits shifted
	and.w	&0xf800,%d2	# remov sign plus exponent
	or.w	%d5,%d2	# put shifted bits in %d2
	eor.w	%d5,%d3	# remov shifted bits from %d3
	mov.l	%d0,u+2(%fp)	# upper half of dividend to local area
	mov.l	%d1,u+4+2(%fp)	# lower half of dividend to local area
	mov.l	&0,%d4	# zero rest of dividend
	mov.w	%d4,u(%fp)	# set u(0) =0
	mov.w	%d4,zero(%fp)	# set v(0) =0
	mov.l	%d4,u+8+2(%fp)	
	mov.l	%d4,u+12+2(%fp)	
	mov.l	%d2,v(%fp)	# upper half of divisor to local area
	mov.l	%d3,v+4(%fp)	# lower half of divisor to local area
 
# step "%d6"        ( calculate q_hat  )
#
# %d4 = 0, will be used as a loop counter and index (j in step %d6 )
# note:  %d4 will be indexed by 2's (words are being indexed)
#
	mov.w	%d2,%d3	# put v(2) in %d3
	clr.w	%d2	# remov v(2)
	swap	%d2	# put v(1) in lower word
#
#        the first quotient bit can only be 0 or 1.
# so set q_hat to 1 and skip the overhead of calculating q_hat and mul.
#
	mov.l	&0,%d1	# %d1=q_hat=1 ...
	add.w	&1,%d1	# with extend=0 (quicker than and to ccr)
	lea	v+6+2(%fp),%a2	# %a2-> ls word of v {plus 2}
	lea	u+8+2(%fp),%a1	# %a1-> u(j+n)  {plus 2}
	subx.l	-(%a2),-(%a1)	
	subx.l	-(%a2),-(%a1)	
	subx.w	-(%a2),-(%a1)	
	bra.b	L%stepd5	# now check if 1 is too big
 
L%d2loop:	mov.w	u(%fp,%d4.w),%d5	# %d5= u(j)
	cmp.w	%d5,%d2	# u(j) = v(1) ???
	bne.b	L%not64k	# if so,set q_hat= 65535
	mov.l	&0xffff,%d1	# set q_hat =64k
	bra.b	L%stepd4	
#
#        unlike knuth, we do not attempt to refine the trial quotient.
#        it is faster to add back if the guess was off by two (or one).
#
L%not64k:	mov.l	u(%fp,%d4.w),%d1	# %d1= u(j)*b + u(j+1)
	divu	%d2,%d1	# %d1=floor{ (u(j)*b + u(j+1)) / v(1) } =q_hat
#**not from knuth*** if q_hat is a 0 the quotient word is zero
	beq.b	L%stepd7	# so skip the multiply and subtract
 
L%stepd4:	# first half of %d0 is:  q_hat * divisor
	lea	u+8+2(%fp,%d4.w),%a2	# %a2-> ls word of u {plus 2}
	mov.l	&0,%d5	# also use %d5 for "carry" word
	mov.l	&0,%d7	# a zeroed register (for subtracting 0 - x bit)
	lea	v+6+2(%fp),%a1	# ls word of v        (plus 2)
	mov.l	&3,%d6	# loop counter
#
#        the following performs a combined multiply and subtract
#
L%mloop:	# warning *** trust me or you'll go crazy
	mov.w	-(%a1),%d0	# get v(x)
	mulu	%d1,%d0	# %d0=q_hat * v(x)
	add.l	%d0,%d5	# add to old carry to form product & carry
	sub.w	%d5,-(%a2)	# subtract product from u (remember borrow)
	clr.w	%d5	# leave only "carry bits"
	swap	%d5	# carry / 64k
	addx.w	%d7,%d5	# include borrow in next subtract
	dbra	%d6,L%mloop	# compute more partial product
	sub.w	%d5,-(%a2)	
#
L%stepd5:	# if	<cs>	then.s      if borrow then add back v
	bcc.b	L%endi1
#
#        unlike knuth,  we repeat the "add back" up to 2 times
#
#	repeat
L%rep1:
L%stepd6:	sub.w	&1,%d1	# dec quotient word
	lea	v+6+2(%fp),%a2	# %a2-> ls word of v {plus 2}
	lea	u+8+2(%fp,%d4.w),%a1	# %a1-> u(j+n)  {plus 2}
	addx.l	-(%a2),-(%a1)	
	addx.l	-(%a2),-(%a1)	
	addx.w	-(%a2),-(%a1)	
#	until.s	<cs>	
	bcc.b	L%rep1
#	endi
L%endi1:
 
L%stepd7:	mov.w	%d1,quotient(%fp,%d4.w)	# save as quotient word
	add.w	&2,%d4	# increment j
	cmp.w	%d4,&8	
	ble	L%d2loop	
L%stepd8:	mov.l	quotient+2(%fp),%d0	# retrieve results
	mov.l	quotient+6(%fp),%d1	
	tst.w	quotient(%fp)	
	# if	<ne>	then.s            shift in 'units' quotient bit
	beq.b	L%else2
	lsr.l	&1,%d0	
	roxr.l	&1,%d1	
	bset	&31,%d0	
	bra.b	L%endi2
#	else.s
L%else2:
# decrement exponent because the result is fractional (also don't need the
# units bit shifted)
	mov.w	sexpq(%fp),%d4	# get sign + exp
	sub.w	&1,%d4	# decrement the exp
	mov.w	%d4,sexpq(%fp)	# store back
	and.w	&0x7ff,%d4	# see if unnormalized now
	beq	L%iszero	# too small, return 0
#	endi
L%endi2:
 
# end knuth
#
# now round up using the highest order bit about to be lost
#
	mov.l	&0,%d4		# can't addx immediate 0
	add.l	&0x400,%d1	# round lower words
	addx.l	%d4,%d0		# round upper words (propagate carry)
#
# note: previous add can't overflow since a quotient of all one's only
# occurs for FFFF... / 8000... which has no bit to round up anyway
#
L%nornd:	rol.l	&5,%d0	# renormalize to floating point format
	swap	%d0	# shift right 11 bits
	rol.l	&5,%d1	
	and.w	&0x001f,%d1	# remov "lost bits"
	swap	%d1	
	mov.l	%d0,%d5	# copy shifted bits
	and.l	&0xffe00000,%d5	# mask for bits shifted
	or.l	%d5,%d1	# put shifted bits in %d1
	and.l	&0x000fffff,%d0	# remov hidden bit & extras
	mov.l	&0,%d4	
	mov.w	sexpq(%fp),%d4	# get sign + exp
	lsl.w	&4,%d4	# adjust for ieee format
	swap	%d4	
	or.l	%d4,%d0	# put in sign+exponent
 
L%done:	# clean up stack and return
	movm.l	(%sp),&reglist	# pop nonvolatile regs
	unlk	%fp
	rts
 
L%zdiv:	# divide by zero error
L%naninf:	# one operand is a nan or infinity
	mov.l	&0x7ff00000,%d0	# return + infinity
	mov.l	&0,%d1
	bra.b	L%done
 
L%iszero:	# result is unnormalized, return a zero
	mov.l	&0,%d0	
	mov.l	&0,%d1	
	bra.b	L%done	
 
#	end
