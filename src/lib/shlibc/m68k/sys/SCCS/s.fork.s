h34991
s 00025/00000/00000
d D 1.1 86/07/31 12:30:02 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
#	@(#)fork.s	6.1 
# C library -- fork

# pid = fork();

# %d1 == 0 in parent process, d1 == 1 in child process.
# %d0 == pid of child in parent, d0 == pid of parent in child.

	file	"fork.s"
	set	fork%,2
	global	fork
	global	cerror%

fork:
	MCOUNT
	mov.l	&fork%,%d0
	trap	&0
	bcc.b	forkok
	jmp	cerror%
forkok:
	tst.b	%d1
	beq.b	parent
	mov.l	&0,%d0
parent:
	rts
E 1
