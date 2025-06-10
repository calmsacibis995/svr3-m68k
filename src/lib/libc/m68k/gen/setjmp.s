#	@(#)setjmp.s	6.1	
# C library -- setjmp, longjmp

#	longjmp(env,val)
# will	generate a "return(val)" from
# the last call to
#	setjmp(env)
# by restoring a1-a7,d2-d7 from 'env'
# and doing a return.
#

	file	"setjmp.s"
	global	setjmp
	global	longjmp

setjmp:
	MCOUNT
	mov.l	(%sp)+,%a1	# pop return pc into %a1
	mov.l	(%sp),%a0	# env
	movm.l	&0xfefc,(%a0)	# save a1-a7, d2-d7 in env
	clr.l	%d0		# return 0
	jmp	(%a1)

longjmp:
	MCOUNT
	add.l	&4,%sp		# throw away this return pc
	mov.l	(%sp)+,%a0	# env
	mov.l	(%sp),%d0	# val
	bne.b	L0
	mov.l	&1,%d0		# force return value non-zero
L0:
	mov.l	24(%a0),%a1	# get return address
	cmp.l	%fp,44(%a0)	# cmp stored %fp and current %fp
	bne.b	L1
	add.l	&48,%a0		# if called within same procedure
				# don't restore registers
	bra.b	L2

L1:
	movm.l	(%a0)+,&0x7efc	# restore a1-a6, d2-d7 from env
L2:
	mov.l	(%a0),%sp
	jmp	(%a1)		# return val (but as if from setjmp)
