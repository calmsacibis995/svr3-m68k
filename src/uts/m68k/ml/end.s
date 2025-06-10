#		Copyright (c) 1985 AT&T		#
#		All Rights Reserved		#
#	@(#)end.m4	6.3	

# MC68020 version
# This	is the code executed by proc(1) to load /etc/init.

	data
	global	icode		# location of boot code
	global	szicode		# size of icode in bytes

# Bootstrap program executed in user mode
# to bring up the system.

	set	exec%,11

icode:	mov.l	&I_stack,%sp
	mov.l	&exec%,%d0
	trap	&0			# exec( "/etc/init" );
L%here:	bra.b	L%here

L%arg:	long	I_ifile
	long	0
L%ifile:
	byte	'/,'e,'t,'c,'/,'i,'n,'i,'t,0
	even
L%stack:
	long	0
	long	I_ifile
	long	L%arg-icode
szicode:
	long	szicode-icode		# size of init code

	set	I_stack,L%stack-icode
	set	I_ifile,L%ifile-icode
