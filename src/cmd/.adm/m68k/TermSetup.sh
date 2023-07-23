# General setup procedure for any terminals.
#   Copyright (c) 1986 by Motorola, Inc.
#
# Invoke from within each user's .profile as follows:
#	TermSetup	
# The sequence sent when so invoked is:
#	is2, tbc, hts, if, is3
# If there is an iprog, it is always run.  If there is an iprog AND an is1,
# then iprog is always run, and is1 is sent ONLY if iprog's exit code = 0.
# This is not exactly how the terminfo manual says we should do it, but it
# provides conditional behavior for downloaded terminals.
#
#  To force is1 to be sent when there is no iprog, invoke as:
#	TermSetup is1
# 
# The is1 string often glitches the screen, for instance clearing it or some
# other thing that may objectionable at every login.
#
#  The TERM environment variable must be properly set first, or the terminal
#  actually connected may receive garbage.

# Proper sequence is: iprog, is1, is2, tbc, hts, if, is3


# # # # #  Begin standard terminal-setup sequences  # # # # #

# If iprog, run it.  Also send is1 if iprog succeeds.
TheProg=`tput iprog`	# get program to run
[ -n "$TheProg" ]  &&  {
	$TheProg  &&  tput is1		# if TheProg works, send is1, too.
}

# If no iprog, but specifically requested, send is1.
[ -z "$TheProg"  -a  "$1" = is1 ]  &&  tput is1

tput is2

# This hack is to avoid a bug in th vt100 description, which will presumably
# be fixed at some future date.
case "$TERM" in
	vt[12]* )	;;		# don't send tab stuff...
	* )		tput tbc
			tput hts  ;;	# everyone else gets tab stuff...
esac

# How we would do it normally, sans vt100 hack...
# tput tbc
# tput hts

TheFile=`tput if`
[ "$TheFile" ]  &&  {
	cat $TheFile;	# send the contents of file
}

tput is3
