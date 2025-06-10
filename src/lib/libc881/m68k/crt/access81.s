#	@(#)access81.s	6.1	

	file	"access81.s"

#	The following routines provide access from C to the system
#	registers for the MC68881 floating point chip.
#
# long rd881_status()
# {
	text
	global	rd881_status
rd881_status:
	link.l	%fp,&-4
	fmovm.l	%status,%d0
# }
	unlk	%fp
	rts

#
# long rd881_iaddr()
# {
	text
	global	rd881_iaddr
rd881_iaddr:
	link.l	%fp,&-4
	fmovm.l	%iaddr,%d0
# }
	unlk	%fp
	rts

#
# long rd881_control()
# {
	text
	global	rd881_control
rd881_control:
	link.l	%fp,&-4
	fmovm.l	%control,%d0
# }
	unlk	%fp
	rts

#
# void wr881_control(newvalue)
# long new_value;
# {
	text
	global	wr881_control
wr881_control:
	link.l	%fp,&-4
	fmovm.l	8(%fp),%control
# }
	unlk	%fp
	rts

