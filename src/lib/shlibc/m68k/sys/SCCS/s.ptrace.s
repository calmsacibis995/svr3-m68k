h26316
s 00018/00000/00000
d D 1.1 86/07/31 12:30:14 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
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
E 1
