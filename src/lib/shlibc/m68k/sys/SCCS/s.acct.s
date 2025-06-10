h21451
s 00016/00000/00000
d D 1.1 86/07/31 12:29:55 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
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
E 1
