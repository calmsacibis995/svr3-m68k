#%W%	%Q%	%G%
##MH************************ MODULE HEADER ****************************
##
##  MODULE NAME:	ci_vmexfer.s
##
##  DESCRIPTION:	This module contains the functions to do
##			VME shared memory to/from MVME131 copying.
##
##  CONTENTS:		The following functions are contained in
##			this module:
##
##			copy_pts	 - copy from private memory to
##					   shared memory
##
##			copy_stp	 - copy from shared memory to
##					   private memory
##
##			get_vme_address  - return MAPPED local address
##					   of a global VME data object.
##
##  MODULE GLOBALS:
##
##  UPDATES:
##
##  Name	Date	  Rev	Comments
##  ----------	--------  ---	--------------------------------------
##  Bill V.	05/09/86  1.0	Copied from vme333 file and revised to
##				 conform to VME131 requirements.
##********************************************************************/

	file	"ci_vmexfer.s"	# This is who we are.

	global	get_vme_address 	# Make available to all.
	global	copy_pts		# Ditto
	global	copy_stp		# Ditto

	set	ARG0,	4	# First argument stack offset.
	set	ARG1,	8	# Second argument stack offset.
	set	ARG2,	12	# Third argument stack offset.

	text			# Start text segment.

#  Copy from private to shared memory 

copy_pts:
	mov.l	ARG1(%sp),%a1		# Argument 1, dest. address.
	mov.l	%a1,-(%sp)		# Pass dest addr to get_vme_address()
	bsr	get_vme_address		# Map dest addr to board's window
	add.l	&4,%sp			# Restore original stack pointer
	mov.l	%a0,%a1			# Get mapped address in dest reg.
	mov.l	ARG0(%sp),%a0		# Argument 0, source address.
	mov.l	ARG2(%sp),%d0		# Argument 2, byte count.
	bra.b	cp_loop			# Go to common copy loop

#  Copy from shared to private memory

copy_stp:
	mov.l	ARG0(%sp),%a0		# Argument 0, source address.
	mov.l	%a0,-(%sp)		# Pass src. addr to get_vme_address()
	bsr	get_vme_address		# Map src. addr to board's window
	add.l	&4,%sp			# Restore original stack pointer
	mov.l	ARG1(%sp),%a1		# Argument 1, dest. address.
	mov.l	ARG2(%sp),%d0		# Argument 2, byte count.

cp_loop: 
	mov.b	(%a0)+,(%a1)+		# Copy byte from source to destination
	sub.l	&1,%d0			# Decrement Counter 
	bne	cp_loop			# Loop Until Done
	rts				# Exit

#  Return real VME address as the local mapped address in a0 and d0, 
#   since VME131 has direct access to shared memory.

get_vme_address:
	
	mov.l	ARG0(%sp),%d0		# get passed address in work reg.
	mov.l	%d0,%a0			# return mapped address in a0 & d0
	rts
