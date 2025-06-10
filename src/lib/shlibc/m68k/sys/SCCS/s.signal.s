h46876
s 00025/00000/00000
d D 1.1 86/07/31 12:30:18 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
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
E 1
