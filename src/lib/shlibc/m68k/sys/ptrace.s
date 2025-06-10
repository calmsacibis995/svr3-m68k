#	@(#)ptrace.s	6.1 
# ptrace -- C library

#	result = ptrace(req, pid, addr, data);

	file	"ptrace.s"
	set	ptrace%,26
	global	ptrace
	global	cerror%

ptrace:
	MCOUNT
	mov.l	&ptrace%,%d0
	trap	&0
	bcc.b 	noerror
	jmp 	cerror%
noerror:
	rts
