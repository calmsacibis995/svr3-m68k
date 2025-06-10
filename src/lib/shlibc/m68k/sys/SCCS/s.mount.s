h26040
s 00019/00000/00000
d D 1.1 86/07/31 12:30:10 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
#	@(#)mount.s	6.1 
# C library -- mount

# error = mount(dev, file, flag)

	file	"mount.s"
	set	mount%,21
	global	mount
	global  cerror%

mount:
	MCOUNT
	mov.l	&mount%,%d0
	trap	&0
	bcc.b 	noerror
	jmp 	cerror%
noerror:
	mov.l	&0,%d0
	rts
E 1
