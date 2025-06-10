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

