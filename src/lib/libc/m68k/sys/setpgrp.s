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
