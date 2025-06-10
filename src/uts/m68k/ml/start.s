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
