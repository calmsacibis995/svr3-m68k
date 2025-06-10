h38242
s 00019/00000/00000
d D 1.1 86/07/31 12:14:34 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
#	@(#)initfpu.s	6.1	

	file	"initfpu.s"
	set	F%1,-4
	text
#
# This routine does nothing useful, since we have no default 
# floating point unit setup to do.  However, the user can
# replace this routine with another routine if he wishes a
# different initialization for the fpu.
#
# void initfpu()
# {
	global	initfpu
initfpu:
	link.l	%fp,&F%1
	unlk	%fp
	rts
# }
E 1
