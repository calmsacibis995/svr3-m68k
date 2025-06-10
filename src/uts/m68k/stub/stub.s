#		Copyright (c) 1985 AT&T		#
#		All Rights Reserved		#
#	@(#)psl.m	1.0	
#
#	processor status register for 68020
#

	set	PS_C,0x0001		# carry bit 
	set	PS_V,0x0002		# overflow bit 
	set	PS_Z,0x0004		# zero bit 
	set	PS_N,0x0008		# negative bit 
	set	PS_EXT,0x0010		# extend bit 

	set	PS_IPL,0x0700		# interrupt priority level 
	set	PS_M,0x1000		# Master/interrupt state 
	set	PS_CUR,0x2000		# current mode. Supervisor if set 
	set	PS_T0,0x4000		# trace enable bit 0 
	set	PS_T1,0x8000		# trace enable bit 1 
	set	PS_T,PS_T1		# trace enable bit 1 

#		Copyright (c) 1985 AT&T		#
#		All Rights Reserved		#
#	@(#)equates.m	1.0	

#  General equate file.
#  This file contains equates which are used throughout the machine
# language routines.  Any equate of this nature should be placed in
# this file.  Equates that are specific to a single machine language
# source file should be defined in the source file that contains them.

set	allfpi,0xff		# Floating point register save/restore mask.
set	savmsk,0xfcff		# this mask determines which registers are saved
set	regsize,14*4		# Size of mem required to save them.
				#  Currently this mask causes D0..D7 and A2..A7.

set	u,0x1000000		# Address of U-block
	global u		# Define the U-block for the entire system

#
#  Cache control functions.
#

set	CACHDIS,8		# Cache diable and flush
set	CACHENB,9		# Cache enable and flush

set	BTOL,2			# Number of shifts for bytes to longs.

#  Ublock equates.  The following may need changing when the user.h
# file is changed.

set	u_procp,0x1bc
set	u_caddrflt,0x70
set	u_prof,0x248

#  Proc equates.  The following may need changing when proc.h changes.
#

set	p_sysid,0x64
#	Stub routine for distributed unix - streams #
#	@(#)stream.s	

	text
strgetmsg:	global	strgetmsg	#
strputmsg:	global	strputmsg	#
sealloc:	global	sealloc		#
sefree:		global	sefree		#
strpoll:	global	strpoll		#
stropen:	global	stropen		#
strread:	global	strread		#
strwrite:	global	strwrite	#
strioctl:	global	strioctl	#
strclean:	global	strclean	#
strclose:	global	strclose	#
runqueues:	global	runqueues	#
	rts

	data
	global	qrunflag
	global	msglistcnt
qrunflag:
msglistcnt:
	long	0			# MUST be zero


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

