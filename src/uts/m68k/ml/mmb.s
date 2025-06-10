#		Copyright (c) 1985 AT&T		#
#		All Rights Reserved		#
#	@(#)mmb.s	6.10	
#
# mmbstore(reg, val)		/* store to mmu register */
# register char *reg;	
# register int val;	
	data
	global mmbad
mmbad:	long 0x10000
	text
	global	mmbstore
mmbstore:
	link	%fp,&0
	tst.w	do_pmmu
	bne	pmmustore
	mov.l	8(%fp),%a1
	mov.l	12(%fp),%d1
#
	add.l	mmbad,%a1	# MMU access address
	mov.l	%dfc,%a0
	mov.l	&7,%d0		# CPU space function code
	mov.l	%d0,%dfc
	movs.l	%d1,(%a1)	# move to MMU register 
	mov.l	%a0,%dfc
L%189:
	unlk	%fp
	rts

pmmustore:
	mov.l	12(%fp),%d1	# Get parameter "val"
	mov.l	8(%fp),%d0	#Get "reg" in d0.

	beq	rpreg		# Should we write TC register?
	nop
	short	0xf001		# "pmove.l %d1,tc"
	short	0x4000
	bra	mmbrts

rpreg:
	mov.l	%d1,rpstore+4	# Write root pointer.
	
	nop
	short	0xf039		# "pmove rpstore,crp"
	short	0x4c00
	long	rpstore
	nop
	short	0xf000		# "pflusha"
	short	0x2400

mmbrts:
	unlk	%fp
	rts

#  This is a work around for mask m00a99a.
	data

trp:
	long	0x7fff0002
	long	0x05010

	text

	global	wakemmu
wakemmu:
	tst.w	do_pmmu
	beq	wakeret
	movm.l	&0x0340,-(%sp)		# Save registers.

	mov.l	&7,%d7
	mov.l	%d7,%dfc
	mov.l	%d7,%sfc
	clr.l	%d6
	movs.l	%d6,0x20010

	movs.w	%d6,0x20002

	lea	trp,%a1

	nop
	long	0xf0114c00

	nop
	short	0xf127
	mov.l	&0x2211,0x18(%a7)
	mov.l	0x1c(%a7),%d6

	and.l	&0xff100380,%d6
	or.l	&0x006e5420,%d6
	mov.l	%d6,0x1c(%a7)

	nop
	short	0xf15f

	movs.w	0x20000,%d6

	nop
	long	0xf0072400

	nop
	short	0xf127

	mov.l	&0x4c00,0x18(%a7)
	mov.l	0x1c(%a7),%d6
	and.l	&0xff300380,%d6
	or.l	&0x00425020,%d6
	mov.l	%d6,0x1c(%a7)

	nop
	short	0xf15f

	movs.w	0x20000,%d6

	lea	trp,%a1

	nop
	long	0xf0116000

	mov.l	&UD,%d7
	mov.l	%d7,%sfc
	mov.l	%d7,%dfc

	movm.l	(%sp)+,&0x02c0
wakeret:
	rts


#
#  The following routine sets the global flag "do_pmmu" to indicate
# whether or not a PMMU was found.
chk_mmu: global	chk_mmu

	mov.l	%vbr,%a0	# Save F line handler
	mov.l	0x2C(%a0),-(%sp)
	mov.l	&chk_mmu6,0x2c(%a0)

#	pflusha
	short	0xf000
	short	0x2400

	mov.w	&1,do_pmmu
chk_mmu4:
	mov.l	(%sp)+,0x2c(%a0)	# Restore old handler.
	rts

chk_mmu6:
	clr.w	do_pmmu
	mov.l	&chk_mmu4,2(%sp)
	rte

	data
	global	do_pmmu
do_pmmu:
	short	0		# Set to indicate pmmu present.

	global	rpstore
rpstore:
	long	0x7fff0003
	long	0
	
	text

