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
