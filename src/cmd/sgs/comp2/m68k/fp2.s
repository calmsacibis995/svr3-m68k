	file	""
	data	1
	text
	global	main
main:
	link.l	%fp,&F%1
	movm.l	&M%1,(4,%sp)
	data	1
	even
L%14:
	long	0x42f6e979	# 1.234560e+02
	text
	mov.l	L%14,((S%1-4).w,%fp)
	data	1
	even
L%15:
	long	0x42f80000	# 1.240000e+02
	text
	mov.l	L%15,((S%1-8).w,%fp)
	data	2
L%16:
	byte	'1,'2,'4,'.,'0,0x00
	text
	mov.l	&L%16,(%sp)
	jsr	atof
	jsr	dbtofl%%
	mov.l	%d0,((S%1-8).w,%fp)
	mov.l	((S%1-8).w,%fp),%d0
	jsr	fltodb%%
	mov.l	%d0,(T%1-32,%fp)
	mov.l	%d1,(T%1-28,%fp)
	mov.l	((S%1-4).w,%fp),%d0
	jsr	fltodb%%
	mov.l	(T%1-28,%fp),(%sp)
	mov.l	(T%1-32,%fp),-(%sp)
	jsr	dbcmp%%
	add.l	&4,%sp
	tst.w	%d0
	bge	L%17
	data	2
L%19:
	byte	'a,0x20,'(,'%,'e,'),0x20,'<
	byte	0x20,'b,0x20,'(,'%,'e,'),'\n
	byte	0x00
	text
	mov.l	((S%1-8).w,%fp),%d0
	jsr	fltodb%%
	mov.l	%d0,(T%1-32,%fp)
	mov.l	%d1,(T%1-28,%fp)
	mov.l	((S%1-4).w,%fp),%d0
	jsr	fltodb%%
	mov.l	%d0,(T%1-40,%fp)
	mov.l	%d1,(T%1-36,%fp)
	mov.l	(T%1-28,%fp),(%sp)
	mov.l	(T%1-32,%fp),-(%sp)
	mov.l	(T%1-36,%fp),-(%sp)
	mov.l	(T%1-40,%fp),-(%sp)
	mov.l	&L%19,-(%sp)
	jsr	printf
	add.l	&16,%sp
L%17:
	mov.l	((S%1-8).w,%fp),%d0
	jsr	fltodb%%
	mov.l	%d0,(T%1-32,%fp)
	mov.l	%d1,(T%1-28,%fp)
	mov.l	((S%1-4).w,%fp),%d0
	jsr	fltodb%%
	mov.l	(T%1-28,%fp),(%sp)
	mov.l	(T%1-32,%fp),-(%sp)
	jsr	dbcmp%%
	add.l	&4,%sp
	tst.w	%d0
	ble	L%20
	data	2
L%21:
	byte	'a,0x20,'(,'%,'f,'),0x20,'>
	byte	0x20,'b,0x20,'(,'%,'f,'),'\n
	byte	0x00
	text
	mov.l	((S%1-8).w,%fp),%d0
	jsr	fltodb%%
	mov.l	%d0,(T%1-32,%fp)
	mov.l	%d1,(T%1-28,%fp)
	mov.l	((S%1-4).w,%fp),%d0
	jsr	fltodb%%
	mov.l	%d0,(T%1-40,%fp)
	mov.l	%d1,(T%1-36,%fp)
	mov.l	(T%1-28,%fp),(%sp)
	mov.l	(T%1-32,%fp),-(%sp)
	mov.l	(T%1-36,%fp),-(%sp)
	mov.l	(T%1-40,%fp),-(%sp)
	mov.l	&L%21,-(%sp)
	jsr	printf
	add.l	&16,%sp
L%20:
	data	1
	even
L%22:
	long	0x405edd2f	# 1.234560e+02
	long	0x1a9fbe77
	text
	mov.l	L%22,((S%1-16).w,%fp)
	mov.l	4+L%22,((S%1-12).w,%fp)
	data	1
	even
L%23:
	long	0x405f0000	# 1.240000e+02
	long	0x0
	text
	mov.l	L%23,((S%1-24).w,%fp)
	mov.l	4+L%23,((S%1-20).w,%fp)
	data	2
L%24:
	byte	'1,'.,'2,'3,'4,'5,'6,'e
	byte	'+,'0,'2,0x00
	text
	mov.l	&L%24,(%sp)
	jsr	atof
	mov.l	%d0,((S%1-16).w,%fp)
	mov.l	%d1,((S%1-12).w,%fp)
	mov.l	((S%1-20).w,%fp),(%sp)
	mov.l	((S%1-24).w,%fp),-(%sp)
	mov.l	((S%1-16).w,%fp),%d0
	mov.l	((S%1-12).w,%fp),%d1
	jsr	dbcmp%%
	add.l	&4,%sp
	tst.w	%d0
	bge	L%25
	data	2
L%26:
	byte	'c,0x20,'(,'%,'e,'),0x20,'<
	byte	0x20,'d,0x20,'(,'%,'e,'),'\n
	byte	0x00
	text
	mov.l	((S%1-20).w,%fp),(%sp)
	mov.l	((S%1-24).w,%fp),-(%sp)
	mov.l	((S%1-12).w,%fp),-(%sp)
	mov.l	((S%1-16).w,%fp),-(%sp)
	mov.l	&L%26,-(%sp)
	jsr	printf
	add.l	&16,%sp
L%25:
	mov.l	((S%1-20).w,%fp),(%sp)
	mov.l	((S%1-24).w,%fp),-(%sp)
	mov.l	((S%1-16).w,%fp),%d0
	mov.l	((S%1-12).w,%fp),%d1
	jsr	dbcmp%%
	add.l	&4,%sp
	tst.w	%d0
	ble	L%27
	data	2
L%28:
	byte	'c,0x20,'(,'%,'e,'),0x20,'>
	byte	0x20,'d,0x20,'(,'%,'e,'),'\n
	byte	0x00
	text
	mov.l	((S%1-20).w,%fp),(%sp)
	mov.l	((S%1-24).w,%fp),-(%sp)
	mov.l	((S%1-12).w,%fp),-(%sp)
	mov.l	((S%1-16).w,%fp),-(%sp)
	mov.l	&L%28,-(%sp)
	jsr	printf
	add.l	&16,%sp
L%27:
L%12:
	movm.l	(4,%sp),&M%1
	unlk	%fp
	rts
	set	S%1,0
	set	T%1,0
	set	F%1,-44
	set	M%1,0x0000
	data	1
