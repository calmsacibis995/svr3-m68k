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
