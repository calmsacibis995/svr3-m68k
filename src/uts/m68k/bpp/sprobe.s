#	@(#)start.m4	6.8	

# MC68020 version

	set	GRAIN,1024		# 1 core unit granularity
	set	ROUND,GRAIN-1		# granularity round
	set	GMASK,-GRAIN		# granularity mask
	set	U_SIZE,2048		# user area for proc 0
	set	HIPRI,0x2700		# supervisor state, ipl 7
	set	LOPRI,0x2000		# supervisor state, ipl 0

	set	USIZE,2048	# number of bytes in the static u area
	set	HIGHPRI,0x2700	# supervisor state priority 7
	set	REGMASK,0xfefc	# save/restore a7-a1,d7-d2
	set	SP,48		# offset into savearea of SP

	set	HIGH,0x2700	# disable all interrupts

	set	OKINTS,0x2000	# enable interrupts
	set	NOINTS,0x2700	# no interrupts

vbr:	long 	0		# contents of the vector base register

# int sprobe( addr, byte )
# caddr_t addr;
# int byte;

# if "byte" < 0, read the short at "addr" to see if a bus error occurs.
# else write (short) "byte" into address "addr" and see if a bus error occurs.

# return true (1) if the error occurs, else false (0).

sprobe:	global	sprobe
	mov.l	%vbr,%d0
	mov.l	%d0,vbr
	mov.l	4(%sp),%a0	# "addr"
	mov.w	%sr,%d1 	# save status register
	mov.w	&NOINTS,%sr	# block interrupts
	mov.l	(8,[vbr]),%a1	# save current bus error handler
	mov.l	&L%spr9,(8,[vbr])	# catch bus error exceptions
	mov.l	8(%sp),%d0	# get the int "byte"
	bge.b	L%spr1		# skip for write
	tst.w	(%a0)		# read (short) "byte" at "addr"
	bra.b	L%spr2
L%spr1:	mov.b	%d0,(%a0)	# write (short) "byte" into "addr"
L%spr2:	mov.l	&0,%d0		# false return value
L%spr3:	mov.l	%a1,(8,[vbr])	# restore bus error handler
	mov.w	%d1,%sr		# restore status register
	rts

# L%spr9 can be reached only by a bus error exception trap

L%spr9:		mov.w	6(%sp),%d0	# get the stack format
	lsr.w	&12,%d0		# shift to low order byte
	mov.b	(stkfmtsz,%d0.w),%d0	# look up size of the saved stack
	add.w	&8,%d0		#  adjust to full saved stack size
	add.w	%d0,%sp		# adjust stack to remove
		# true return value
	mov.l	&1,%d0		# true return value
	bra.b	L%spr3

