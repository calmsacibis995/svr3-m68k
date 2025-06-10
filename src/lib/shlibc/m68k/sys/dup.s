#	@(#)dup.s	6.1 
# C library -- dup

#	f = dup(of [ ,nf])
#	f == -1 for error

	file	"dup.s"
	set	dup%,41
	global	dup
	global	cerror%

dup:
	MCOUNT
	mov.l	&dup%,%d0
	trap	&0
	bcc.b 	noerror
	jmp 	cerror%
noerror:
	rts
