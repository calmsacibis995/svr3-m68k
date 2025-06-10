#	@(#)signal.s	6.1 
# C library -- _signal

# _signal(n, 0);	/* default action on signal(n) */
# _signal(n, odd);	/* ignore signal(n) */
# _signal(n, label);	/* goto label on signal(n) */
# returns old label, only one level.

# _signal is the true system call.
# signal is really a C interface toutine that calls _signal().

	file	"signal.s"
	set	signal%,48
	global	_signal
	global  cerror%

	even
_signal:
	MCOUNT
	mov.l	&signal%,%d0
	trap	&0
	bcc.b 	noerror
	jmp 	cerror%
noerror:
	rts
