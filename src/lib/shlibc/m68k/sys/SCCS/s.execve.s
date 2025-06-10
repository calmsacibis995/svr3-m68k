h29171
s 00018/00000/00000
d D 1.1 86/07/31 12:30:00 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
#	@(#)execve.s	6.1 
# C library -- execve

# execve(file, argv, env);

# where argv is a vector argv[0] ... argv[x], 0
# last vector element must be 0

	file	"execve.s"
	set	exece%,59
	global	execve
	global	cerror%

execve:
	MCOUNT
	mov.l	&exece%,%d0
	trap	&0
	jmp 	cerror%
E 1
