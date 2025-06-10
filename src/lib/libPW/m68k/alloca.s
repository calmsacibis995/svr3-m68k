#	@(#)alloca.s	6.1 
# like alloc, but automatic
# automatic free in return

	file	"alloca.s"
	global	alloca
alloca:
	mov.l	(%sp)+,%a1	# save return address
	mov.l	(%sp)+,%d0	# get number bytes to allocate
	add.l	&3,%d0		#
	and.l	&-4,%d0		# round amount to long word boundary
	sub.l	%d0,%sp		# allocate on stack
	mov.l	%sp,%a0		# return amount allocated
	sub.l	&4,%sp		# dummy argument will be stripped
	jmp	(%a1)		# funny return
