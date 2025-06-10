# Motorola M68020 interrupt/trap handler

	text
m564int:	global	m564int	# vector address 110
	movm.l	&0xc0c0,-(%sp)
	mov.l	&0,%d0
	mov.l	%d0,-(%sp)
L%m564:	jsr	m564intr
	add.l	&4,%sp
	bra	intret

m355int:	global	m355int	# vector address 11c
	movm.l	&0xc0c0,-(%sp)
	jsr	m355intr
	bra	intret

m323int:	global	m323int	# vector address 324
	movm.l	&0xc0c0,-(%sp)
	mov.l	&0,%d0
	mov.l	%d0,-(%sp)
L%m323:	jsr	m323intr
	add.l	&4,%sp
	bra	intret

m3313int:	global	m3313int	# vector address 3b0
	movm.l	&0xc0c0,-(%sp)
	mov.l	&24,%d0
	mov.l	%d0,-(%sp)
	bra	L%m331

m3312int:	global	m3312int	# vector address 3b4
	movm.l	&0xc0c0,-(%sp)
	mov.l	&16,%d0
	mov.l	%d0,-(%sp)
	bra	L%m331

m3311int:	global	m3311int	# vector address 3b8
	movm.l	&0xc0c0,-(%sp)
	mov.l	&8,%d0
	mov.l	%d0,-(%sp)
	bra	L%m331

m331int:	global	m331int	# vector address 3bc
	movm.l	&0xc0c0,-(%sp)
	mov.l	&0,%d0
	mov.l	%d0,-(%sp)
L%m331:	jsr	m331intr
	add.l	&4,%sp
	bra	intret

m3501int:	global	m3501int	# vector address 3c8
	movm.l	&0xc0c0,-(%sp)
	mov.l	&1,%d0
	mov.l	%d0,-(%sp)
	bra	L%m350

m350int:	global	m350int	# vector address 3cc
	movm.l	&0xc0c0,-(%sp)
	mov.l	&0,%d0
	mov.l	%d0,-(%sp)
L%m350:	jsr	m350intr
	add.l	&4,%sp
	bra	intret

m3601int:	global	m3601int	# vector address 3dc
	movm.l	&0xc0c0,-(%sp)
	mov.l	&32,%d0
	mov.l	%d0,-(%sp)
	bra	L%m360

m360int:	global	m360int	# vector address 3e4
	movm.l	&0xc0c0,-(%sp)
	mov.l	&0,%d0
	mov.l	%d0,-(%sp)
L%m360:	jsr	m360intr
	add.l	&4,%sp
	bra	intret

m3201int:	global	m3201int	# vector address 3f0
	movm.l	&0xc0c0,-(%sp)
	mov.l	&32,%d0
	mov.l	%d0,-(%sp)
	bra	L%m320

m320int:	global	m320int	# vector address 3f4
	movm.l	&0xc0c0,-(%sp)
	mov.l	&0,%d0
	mov.l	%d0,-(%sp)
L%m320:	jsr	m320intr
	add.l	&4,%sp
	bra	intret

