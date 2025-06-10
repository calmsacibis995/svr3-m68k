h25113
s 00018/00000/00000
d D 1.1 86/07/31 12:30:30 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
#	@(#)utime.s	6.1 
# C library -- utime
 
#  error = utime(string,timev);
 
	file	"utime.s"
	set	utime%,30
	global	utime
	global	cerror%
 
utime:
	MCOUNT
	mov.l	&utime%,%d0
	trap	&0
	bcc.b	noerror
	jmp	cerror%
noerror:
	rts
E 1
