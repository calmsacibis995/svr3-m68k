#		Copyright (c) 1985 Motorola, Inc.   #
#		All Rights Reserved		#
ifdef(`VME131',,
`
<real_dev>	    <alias>	<fsize>	<perms>		<format_pgm >
')dnl

ifdef(`MACS',
`
/dev/dsk/vm21_4s0   default	 988	16x32k:RWF	/etc/dinit
/dev/dsk/vm21_4s0   4      	 988	16x32k:RWF	/etc/dinit
/dev/dsk/vm21_5s0   5      	 988	16x32k:RWF	/etc/dinit
/dev/dsk/vm21_4s7   old		1000	16x32k:RWF	/etc/dinit
')dnl
ifdef(`VM03',
`
/dev/dsk/vm21_4s0   default	 988	16x32k:RWF	/etc/dinit
/dev/dsk/vm21_4s7   old		1000	16x32k:RWF	/etc/dinit
/dev/dsk/vm22_10s2  10     	1264	20x32k:RWF	/etc/dinit
/dev/dsk/vm22_10s7  10old	1276	20x32k:RWF	/etc/dinit
')dnl
ifdef(`VM04',
`
/dev/dsk/vm21_4s0   default	 988	16x32k:RWF	/etc/dinit
/dev/dsk/vm21_4s7   old		1000	16x32k:RWF	/etc/dinit
/dev/dsk/vm22_10s2  10     	1264	20x32k:RWF	/etc/dinit
/dev/dsk/vm22_10s7  10old	1276	20x32k:RWF	/etc/dinit
')dnl
ifdef(`VME10',
`
/dev/dsk/wd_2s0	    default	1264	20x32k:RWF	/etc/wffmt
/dev/dsk/wd_2s7	    old		1276	20x32k:RWF	/etc/wffmt
')dnl
ifdef(`VME120',
`
/dev/dsk/m320_2s0   default	1264	20x32k:RWF	/etc/dinit
/dev/dsk/m320_2s7   old		1276	20x32k:RWF	/etc/dinit
')dnl
ifdef(`VME131',
`
<real_dev>  <alias>	<mntpt>	<fsize>		<perms>	<format_pgm >

/dev/02s0   default	/flp	1264:144	RWF	/etc/fmtflp
/dev/02s0   flp		/flp	1264:144	RWF	/etc/fmtflp
/dev/02s0   bflp	/flp	1264:144	RWF	/etc/fmtbflp
/dev/03s0   flp2	/flp2	1264:144	RWF	/etc/fmtflp
/dev/03s0   bflp2	/flp2	1264:144	RWF	/etc/fmtbflp
/dev/00s2   usr		/usr	84480		R	none
/dev/01s0   w1		/w1	26880		R	none
')dnl
