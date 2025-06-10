#	Stub routine for distributed unix - rfs #
#	@(#)du.s	

	text
rfubyte:	global	rfubyte		#
rfuword:	global	rfuword		#
rsubyte:	global	rsubyte		#
rsuword:	global	rsuword		#
rcopyin:	global	rcopyin		#
rcopyout:	global	rcopyout	#
advfs:		global	advfs		#
unadvfs:	global	unadvfs		#
rmount:		global	rmount		#
rumount:	global	rumount		#
rfstart:	global	rfstart		#
rdebug:		global	rdebug		#
rfstop:		global	rfstop		#
rfsys:		global	rfsys		#
duustat:	global	duustat		#
dulocking:	global	dulocking	#
rnamei1:	global	rnamei1		#
rnamei2:	global	rnamei2		#
rnamei3:	global	rnamei3		#
rnamei4:	global	rnamei4		#
riget:		global	riget		#
remote_call:	global	remote_call	#
remio:		global	remio		#
rem_date:	global	rem_date	#
unremio:	global	unremio		#
	rts

#
#	Data definitions
#
	global	nservers
	global	idleserver
	global	srmount
nservers:
idleserver:
srmount:
	long	0			# MUST be zero

