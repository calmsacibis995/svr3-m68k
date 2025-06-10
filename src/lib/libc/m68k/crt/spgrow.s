#	@(#)spgrow.s	6.1	
#
#	spgrow% - stack grow `C' run-time interface
#
#	code for entry to this routine is automatically generated
#	by the pcc compiler for the 68000.  this is the stack grow
#	routine used to prevent stack overflow on function entry.
#
#	upon entry, %a0 contains the local area req'd + safety factor

	file	"spgrow.s"
	set	spgrow%%,1
	set	SIGSEGV,11
	global	spgrow%

spgrow%:
	mov.l	&splimit%,%a1
	mov.l	%a1,-(%sp)	# arg2
	mov.l	(%a1),%a1
	sub.l	%a0,%a1		# %a0 has F%n - size of local area from pcc
				# previously calculated in func preamble
	mov.l	%a1,-(%sp)	# arg1
	mov.l	&spgrow%%,%d0
	mov.l	%d0,-(%sp)	# cmd
	jsr	sysm68k
	tst.l	%d0
	beq.b	L%noerr
	sub.l	&256-32,4(%sp)	# try again without safety factor
	ble.b	L%noerr		# if resultant incr <= 0 - minimally alright
	jsr	sysm68k
	tst.l	%d0
	beq.b	L%noerr
	mov.l	&SIGSEGV,%d0	# failed again. kill self
	mov.l	%d0,4(%sp)
	jsr	getpid
	mov.l	%d0,(%sp)
	jsr	kill
L%noerr:
	add.l	&12,%sp
	rts
