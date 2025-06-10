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
