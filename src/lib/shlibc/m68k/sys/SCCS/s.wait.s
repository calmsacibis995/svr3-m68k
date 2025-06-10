h39894
s 00028/00000/00000
d D 1.1 86/07/31 12:30:30 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
#	@(#)wait.s	6.1 
# C library -- wait

# pid = wait(0);
#   or,
# pid = wait(&status);

# pid == -1 if error
# status indicates fate of process, if given

	file	"wait.s"
	set	wait%,7
	global	wait
	global  cerror%

wait:
	MCOUNT
	mov.l	&wait%,%d0
	trap	&0
	bcc.b 	noerror
	jmp 	cerror%
noerror:
	tst.l	4(%sp)		# status desired?
	beq.b	nostatus	# no
	mov.l	4(%sp),%a0
	mov.l	%d1,(%a0)	# store child's status
nostatus:
	rts
E 1
