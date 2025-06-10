#	@(#)acct.s	6.1 
# C library -- acct

	file	"acct.s"
	set	acct%,51
	global	acct
	global  cerror%

acct:
	MCOUNT
	mov.l	&acct%,%d0
	trap	&0
	bcc.b 	noerror
	jmp 	cerror%
noerror:
	rts
