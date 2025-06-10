h31800
s 00028/00000/00000
d D 1.1 86/07/31 12:30:17 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
#	@(#)setpgrp.s	6.1 
# C library -- setpgrp, getpgrp

	file	"setpgrp.s"
	set	setpgrp%,39
	global	setpgrp
	global	getpgrp

setpgrp:
	MCOUNT
	mov.l	&1,%d0
	mov.l	%d0,-(%sp)
	bsr.b	pgrp
	add.l	&4,%sp
	rts

getpgrp:
	MCOUNT
	mov.l	&0,%d0
	mov.l	%d0,-(%sp)
	bsr.b	pgrp
	add.l	&4,%sp
	rts

pgrp:
	mov.l	&setpgrp%,%d0
	trap	&0
	rts
E 1
