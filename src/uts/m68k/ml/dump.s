#		Copyright (c) 1985 AT&T		#
#		All Rights Reserved		#
# 	@(#)dump.m4	6.1	

# MC68020 version

	set	HIGH,0x0700	# disable all interrupts
	data
	even
	global	_sdata
_sdata:				# start of data
	space	100*4		# seperate stack for dumps
	global	dumpstk
dumpstk: 
	space	16*4		# registers %d0-%d7, %a0-%a7
	space	4		# status register
	space	4		# user stack pointer
	space	4		# vector base register
	text
# Produce a core image dump
	global	dump%
dump%:
	movm.l	&0xffff,dumpstk	# save registers
	mov.l	&dumpstk,%sp	# reinit stack
	mov.w	%sr,%d0
	or.w	&HIGH,%sr	# disable interrupts
	lea	16*4(%sp),%a1
	mov.l	%d0,(%a1)+	# status register
	mov.l	%usp,%a0
	mov.l	%a0,(%a1)+	# user stack pointer
	mov.l	%vbr,%a0
	mov.l	%a0,(%a1)+	# vector base register
	mov.l	dump,%a0	# get address of configured dump routine
	jsr	(%a0)
L%dump:	stop	&HIGH
	bra.b	L%dump
