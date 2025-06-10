#	@(#)time.s	6.1 
# C library -- time

# tvec = time(tvec);

	file	"time.s"
	set	time%,13
global	time

time:
	MCOUNT
	mov.l	&time%,%d0
	trap	&0
	mov.l	4(%sp),%d1
	beq.b	nostore
	mov.l	%d1,%a0
	mov.l	%d0,(%a0)
nostore:
	rts
