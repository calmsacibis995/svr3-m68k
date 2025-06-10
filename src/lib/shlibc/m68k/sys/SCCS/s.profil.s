h17094
s 00012/00000/00000
d D 1.1 86/07/31 12:30:14 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
#	@(#)profil.s	6.1 
# profil

	file	"profil.s"
	set	prof%,44
	global	profil

profil:
	MCOUNT
	mov.l	&prof%,%d0
	trap	&0
	rts
E 1
