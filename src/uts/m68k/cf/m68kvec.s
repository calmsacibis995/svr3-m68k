# Motorola M68020 vector table

	text
	global	M68Kvec
M68Kvec:
	nop
	jmp	start
	org	8
	long	Xprotflt	# 8 <==
	long	nullvect	# c
	long	instintr	# 10 <==
	long	nullvect	# 14
	long	nullvect	# 18
	long	nullvect	# 1c
	long	nullvect	# 20
	long	trceintr	# 24 <==
	long	nullvect	# 28
	long	nullvect	# 2c
	long	nullvect	# 30
	long	nullvect	# 34
	long	nullvect	# 38
	long	nullvect	# 3c
	long	nullvect	# 40
	long	nullvect	# 44
	long	nullvect	# 48
	long	nullvect	# 4c
	long	nullvect	# 50
	long	nullvect	# 54
	long	nullvect	# 58
	long	nullvect	# 5c
	long	nullvect	# 60
	long	nullvect	# 64
	long	nullvect	# 68
	long	nullvect	# 6c
	long	nullvect	# 70
	long	nullvect	# 74
	long	nullvect	# 78
	long	nullvect	# 7c
	long	Xsyscall	# 80 <==
	long	nullvect	# 84
	long	nullvect	# 88
	long	nullvect	# 8c
	long	nullvect	# 90
	long	nullvect	# 94
	long	nullvect	# 98
	long	nullvect	# 9c
	long	nullvect	# a0
	long	nullvect	# a4
	long	nullvect	# a8
	long	nullvect	# ac
	long	nullvect	# b0
	long	nullvect	# b4
	long	nullvect	# b8
	long	nullvect	# bc
	long	nullvect	# c0
	long	nullvect	# c4
	long	nullvect	# c8
	long	nullvect	# cc
	long	nullvect	# d0
	long	nullvect	# d4
	long	nullvect	# d8
	long	nullvect	# dc
	long	nullvect	# e0
	long	nullvect	# e4
	long	nullvect	# e8
	long	nullvect	# ec
	long	nullvect	# f0
	long	nullvect	# f4
	long	nullvect	# f8
	long	nullvect	# fc
	long	swabintr	# 100 <==
	long	nullvect	# 104
	long	Xclock	# 108 <==
	long	nullvect	# 10c
	long	 m564int	# 110 <==
	long	nullvect	# 114
	long	nullvect	# 118
	long	 m355int	# 11c <==
	long	nullvect	# 120
	long	nullvect	# 124
	long	nullvect	# 128
	long	nullvect	# 12c
	long	nullvect	# 130
	long	nullvect	# 134
	long	nullvect	# 138
	long	nullvect	# 13c
	long	nullvect	# 140
	long	nullvect	# 144
	long	nullvect	# 148
	long	nullvect	# 14c
	long	nullvect	# 150
	long	nullvect	# 154
	long	nullvect	# 158
	long	nullvect	# 15c
	long	nullvect	# 160
	long	nullvect	# 164
	long	nullvect	# 168
	long	nullvect	# 16c
	long	nullvect	# 170
	long	nullvect	# 174
	long	nullvect	# 178
	long	nullvect	# 17c
	long	nullvect	# 180
	long	nullvect	# 184
	long	nullvect	# 188
	long	nullvect	# 18c
	long	nullvect	# 190
	long	nullvect	# 194
	long	nullvect	# 198
	long	nullvect	# 19c
	long	nullvect	# 1a0
	long	nullvect	# 1a4
	long	nullvect	# 1a8
	long	nullvect	# 1ac
	long	nullvect	# 1b0
	long	nullvect	# 1b4
	long	nullvect	# 1b8
	long	nullvect	# 1bc
	long	nullvect	# 1c0
	long	nullvect	# 1c4
	long	nullvect	# 1c8
	long	nullvect	# 1cc
	long	nullvect	# 1d0
	long	nullvect	# 1d4
	long	nullvect	# 1d8
	long	nullvect	# 1dc
	long	nullvect	# 1e0
	long	nullvect	# 1e4
	long	nullvect	# 1e8
	long	nullvect	# 1ec
	long	nullvect	# 1f0
	long	nullvect	# 1f4
	long	nullvect	# 1f8
	long	nullvect	# 1fc
	long	nullvect	# 200
	long	nullvect	# 204
	long	nullvect	# 208
	long	nullvect	# 20c
	long	nullvect	# 210
	long	nullvect	# 214
	long	nullvect	# 218
	long	nullvect	# 21c
	long	nullvect	# 220
	long	nullvect	# 224
	long	nullvect	# 228
	long	nullvect	# 22c
	long	nullvect	# 230
	long	nullvect	# 234
	long	nullvect	# 238
	long	nullvect	# 23c
	long	nullvect	# 240
	long	nullvect	# 244
	long	nullvect	# 248
	long	nullvect	# 24c
	long	nullvect	# 250
	long	nullvect	# 254
	long	nullvect	# 258
	long	nullvect	# 25c
	long	nullvect	# 260
	long	nullvect	# 264
	long	nullvect	# 268
	long	nullvect	# 26c
	long	nullvect	# 270
	long	nullvect	# 274
	long	nullvect	# 278
	long	nullvect	# 27c
	long	nullvect	# 280
	long	nullvect	# 284
	long	nullvect	# 288
	long	nullvect	# 28c
	long	nullvect	# 290
	long	nullvect	# 294
	long	nullvect	# 298
	long	nullvect	# 29c
	long	nullvect	# 2a0
	long	nullvect	# 2a4
	long	nullvect	# 2a8
	long	nullvect	# 2ac
	long	nullvect	# 2b0
	long	nullvect	# 2b4
	long	nullvect	# 2b8
	long	nullvect	# 2bc
	long	nullvect	# 2c0
	long	nullvect	# 2c4
	long	nullvect	# 2c8
	long	nullvect	# 2cc
	long	nullvect	# 2d0
	long	nullvect	# 2d4
	long	nullvect	# 2d8
	long	nullvect	# 2dc
	long	nullvect	# 2e0
	long	nullvect	# 2e4
	long	nullvect	# 2e8
	long	nullvect	# 2ec
	long	nullvect	# 2f0
	long	nullvect	# 2f4
	long	nullvect	# 2f8
	long	nullvect	# 2fc
	long	nullvect	# 300
	long	nullvect	# 304
	long	nullvect	# 308
	long	nullvect	# 30c
	long	nullvect	# 310
	long	nullvect	# 314
	long	nullvect	# 318
	long	nullvect	# 31c
	long	nullvect	# 320
	long	 m323int	# 324 <==
	long	nullvect	# 328
	long	nullvect	# 32c
	long	nullvect	# 330
	long	nullvect	# 334
	long	nullvect	# 338
	long	nullvect	# 33c
	long	nullvect	# 340
	long	nullvect	# 344
	long	nullvect	# 348
	long	nullvect	# 34c
	long	nullvect	# 350
	long	nullvect	# 354
	long	nullvect	# 358
	long	nullvect	# 35c
	long	nullvect	# 360
	long	nullvect	# 364
	long	nullvect	# 368
	long	nullvect	# 36c
	long	nullvect	# 370
	long	nullvect	# 374
	long	nullvect	# 378
	long	nullvect	# 37c
	long	nullvect	# 380
	long	nullvect	# 384
	long	nullvect	# 388
	long	nullvect	# 38c
	long	nullvect	# 390
	long	nullvect	# 394
	long	nullvect	# 398
	long	nullvect	# 39c
	long	nullvect	# 3a0
	long	nullvect	# 3a4
	long	nullvect	# 3a8
	long	nullvect	# 3ac
	long	m3313int	# 3b0 <==
	long	m3312int	# 3b4 <==
	long	m3311int	# 3b8 <==
	long	 m331int	# 3bc <==
	long	nullvect	# 3c0
	long	nullvect	# 3c4
	long	m3501int	# 3c8 <==
	long	 m350int	# 3cc <==
	long	nullvect	# 3d0
	long	nullvect	# 3d4
	long	nullvect	# 3d8
	long	m3601int	# 3dc <==
	long	nullvect	# 3e0
	long	 m360int	# 3e4 <==
	long	nullvect	# 3e8
	long	nullvect	# 3ec
	long	m3201int	# 3f0 <==
	long	 m320int	# 3f4 <==
	long	nullvect	# 3f8
	long	nullvect	# 3fc

	jmp	dump%	 # goto dump()

	data

sysdevs:	global	sysdevs
	short	36
# boot 0 system device data
	short	01000
	short	01000
	short	01400
	short	01001
	long	1
	long	17279
# boot 1 system device data
	short	01010
	short	01010
	short	01400
	short	01011
	long	1
	long	17279
# boot 2 system device data
	short	01020
	short	01020
	short	01400
	short	01020
	long	1245
	long	19
# boot 3 system device data
	short	01030
	short	01030
	short	01400
	short	01030
	long	1245
	long	19
# boot 4 system device data
	short	01100
	short	01100
	short	01400
	short	01101
	long	1
	long	17279
# boot 5 system device data
	short	01110
	short	01110
	short	01400
	short	01111
	long	1
	long	17279
# boot 6 system device data
	short	01120
	short	01120
	short	01400
	short	01120
	long	1245
	long	19
# boot 7 system device data
	short	01130
	short	01130
	short	01400
	short	01130
	long	1245
	long	19
# boot 8 system device data
	short	03000
	short	03000
	short	01400
	short	03000
	long	26880
	long	16000
# boot 9 system device data
	short	03020
	short	03020
	short	01400
	short	03020
	long	26880
	long	16000
# boot 10 system device data
	short	037777777777
	short	037777777777
	short	01400
	short	037777777777
	long	26880
	long	16000
# boot 11 system device data
	short	037777777777
	short	037777777777
	short	01400
	short	037777777777
	long	26880
	long	16000
# boot 12 system device data
	short	03200
	short	03200
	short	01400
	short	03200
	long	26880
	long	16000
# boot 13 system device data
	short	03220
	short	03220
	short	01400
	short	03220
	long	26880
	long	16000
# boot 14 system device data
	short	037777777777
	short	037777777777
	short	01400
	short	037777777777
	long	26880
	long	16000
# boot 15 system device data
	short	037777777777
	short	037777777777
	short	01400
	short	037777777777
	long	26880
	long	16000
# boot 16 system device data
	short	02400
	short	02400
	short	01400
	short	02401
	long	1
	long	200
# boot 17 system device data
	short	037777777777
	short	037777777777
	short	01400
	short	037777777777
	long	1
	long	1
# boot 18 system device data
	short	037777777777
	short	037777777777
	short	01400
	short	037777777777
	long	1
	long	1
# boot 19 system device data
	short	037777777777
	short	037777777777
	short	01400
	short	037777777777
	long	1
	long	1
# boot 20 system device data
	short	02400
	short	02400
	short	01400
	short	02401
	long	1
	long	200
# boot 21 system device data
	short	037777777777
	short	037777777777
	short	01400
	short	037777777777
	long	1
	long	1
# boot 22 system device data
	short	037777777777
	short	037777777777
	short	01400
	short	037777777777
	long	1
	long	1
# boot 23 system device data
	short	037777777777
	short	037777777777
	short	01400
	short	037777777777
	long	1
	long	1
# boot 24 system device data
	short	02400
	short	02400
	short	01400
	short	02401
	long	1
	long	200
# boot 25 system device data
	short	037777777777
	short	037777777777
	short	01400
	short	037777777777
	long	1
	long	1
# boot 26 system device data
	short	037777777777
	short	037777777777
	short	01400
	short	037777777777
	long	1
	long	1
# boot 27 system device data
	short	037777777777
	short	037777777777
	short	01400
	short	037777777777
	long	1
	long	1
# boot 28 system device data
	short	02400
	short	02400
	short	01400
	short	02401
	long	1
	long	200
# boot 29 system device data
	short	037777777777
	short	037777777777
	short	01400
	short	037777777777
	long	1
	long	1
# boot 30 system device data
	short	037777777777
	short	037777777777
	short	01400
	short	037777777777
	long	1
	long	1
# boot 31 system device data
	short	037777777777
	short	037777777777
	short	01400
	short	037777777777
	long	1
	long	1
# boot 32 system device data
	short	03400
	short	03400
	short	01400
	short	03401
	long	1
	long	25199
# boot 33 system device data
	short	03420
	short	03420
	short	01400
	short	03421
	long	1
	long	25199
# boot 34 system device data
	short	03440
	short	03440
	short	01400
	short	03441
	long	1
	long	25199
# boot 35 system device data
	short	03460
	short	03460
	short	01400
	short	03461
	long	1
	long	25199

sysdef1:	global	sysdef1
	byte	'c,'p,'m,'e,'m,'\t,'0,'\t
	byte	'0,'\t,'0,'\t,'1,'\n,'s,'5
	byte	'\t,'0,'\t,'0,'\t,'0,'\t,'1
	byte	'0,'\n,'m,'5,'6,'4,'c,'o
	byte	'n,'\t,'1,'1,'0,'\t,'f,'f
	byte	'f,'b,'0,'0,'4,'0,'\t,'2
	byte	'\t,'2,'\n,'v,'m,'e,'3,'2
	byte	'0,'\t,'3,'f,'4,'\t,'f,'f
	byte	'f,'f,'b,'0,'0,'0,'\t,'5
	byte	'\n,'v,'m,'e,'3,'2,'0,'\t
	byte	'3,'f,'0,'\t,'f,'f,'f,'f
	byte	'a,'c,'0,'0,'\t,'5,'\n,'v
	byte	'm,'e,'3,'2,'3,'\t,'3,'2
	byte	'4,'\t,'f,'f,'f,'f,'a,'0
	byte	'0,'0,'\t,'3,'\n,'v,'m,'e
	byte	'3,'3,'1,'\t,'3,'b,'c,'\t
	byte	'f,'f,'f,'f,'6,'0,'0,'0
	byte	'\t,'2,'\t,'8,'\n,'v,'m,'e
	byte	'3,'3,'1,'\t,'3,'b,'8,'\t
	byte	'f,'f,'f,'f,'6,'1,'0,'0
	byte	'\t,'2,'\t,'8,'\n,'v,'m,'e
	byte	'3,'3,'1,'\t,'3,'b,'4,'\t
	byte	'f,'f,'f,'f,'6,'2,'0,'0
	byte	'\t,'2,'\t,'8,'\n,'v,'m,'e
	byte	'3,'3,'1,'\t,'3,'b,'0,'\t
	byte	'f,'f,'f,'f,'6,'3,'0,'0
	byte	'\t,'2,'\t,'8,'\n,'v,'m,'e
	byte	'3,'5,'0,'\t,'3,'c,'c,'\t
	byte	'f,'f,'f,'f,'5,'0,'0,'0
	byte	'\t,'3,'\n,'v,'m,'e,'3,'5
	byte	'0,'\t,'3,'c,'8,'\t,'f,'f
	byte	'f,'f,'5,'1,'0,'0,'\t,'3
	byte	'\n,'v,'m,'e,'3,'5,'5,'\t
	byte	'1,'1,'c,'\t,'f,'f,'f,'f
	byte	'0,'8,'0,'0,'\t,'5,'\n,'v
	byte	'm,'e,'3,'6,'0,'\t,'3,'e
	byte	'4,'\t,'f,'f,'f,'f,'0,'c
	byte	'0,'0,'\t,'5,'\n,'v,'m,'e
	byte	'3,'6,'0,'\t,'3,'d,'c,'\t
	byte	'f,'f,'f,'f,'0,'e,'0,'0
	byte	'\t,'5,'\n,'m,'s,'g,'\t,'0
	byte	'\t,'0,'\t,'0,'\t,'1,'\n,'r
	byte	'a,'m,'d,'\t,'0,'\t,'0,'\t
	byte	'0,'\n,'s,'x,'t,'\t,'0,'\t
	byte	'0,'\t,'0,'\t,'6,'\n,'s,'e
	byte	'm,'\t,'0,'\t,'0,'\t,'0,'\t
	byte	'1,'\n,'s,'h,'m,'\t,'0,'\t
	byte	'0,'\t,'0,'\t,'1,'\n,'v,'m
	byte	'e,'2,'0,'4,'\t,'0,'\t,'0
	byte	'\t,'0,'\t,'1,'\n,0

sysdef3:	global	sysdef3
	byte	'a,'l,'i,'e,'n,'\t,'1,'0
	byte	'8,'\t,'0,'X,'c,'l,'o,'c
	byte	'k,'\n,'a,'l,'i,'e,'n,'\t
	byte	'8,'\t,'0,'X,'p,'r,'o,'t
	byte	'f,'l,'t,'\n,'a,'l,'i,'e
	byte	'n,'\t,'8,'0,'\t,'0,'X,'s
	byte	'y,'s,'c,'a,'l,'l,'\n,'a
	byte	'l,'i,'e,'n,'\t,'1,'0,'\t
	byte	'0,'i,'n,'s,'t,'i,'n,'t
	byte	'r,'\n,'a,'l,'i,'e,'n,'\t
	byte	'1,'0,'0,'\t,'0,'s,'w,'a
	byte	'b,'i,'n,'t,'r,'\n,'a,'l
	byte	'i,'e,'n,'\t,'2,'4,'\t,'0
	byte	't,'r,'c,'e,'i,'n,'t,'r
	byte	'\n,'d,'u,'p,'\t,'4,'\t,'1
	byte	'0,'8,'\t,'X,'c,'l,'o,'c
	byte	'k,'\n,'d,'u,'p,'\t,'4,'\t
	byte	'8,'\t,'X,'p,'r,'o,'t,'f
	byte	'l,'t,'\n,'d,'u,'p,'\t,'4
	byte	'\t,'8,'0,'\t,'X,'s,'y,'s
	byte	'c,'a,'l,'l,'\n,'d,'u,'p
	byte	'\t,'4,'\t,'1,'0,'\t,'i,'n
	byte	's,'t,'i,'n,'t,'r,'\n,'d
	byte	'u,'p,'\t,'4,'\t,'2,'4,'\t
	byte	't,'r,'c,'e,'i,'n,'t,'r
	byte	'\n,0
