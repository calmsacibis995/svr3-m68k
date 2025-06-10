h23825
s 00019/00000/00000
d D 1.1 86/07/31 12:30:21 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
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
E 1
