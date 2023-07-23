# Set variable ASSUME to be assumed value for TERM
#   Copyright (c) 1986 by Motorola, Inc.
#
# Invoked only from within TermIs.

#  *************************************************************************
# *****  You must edit this file so it correctly describes your system  *****
#  *************************************************************************

# The variable ASSUME should be set to the default assumed terminal name for
# your system.  The variable REMOTE is distinct from ASSUME, in that REMOTE
# is the assumed terminal for remote connections, i.e. not direct connects.
# REMOTE is most useful in conjunction with switched or dial-in terminals from
# different possible locations.


# These are the simplest assumptions...
ASSUME=vt100		# default case for any terminal
REMOTE=vt100		# default case for REMOTE terminals


# Change these sections to reflect the use of serial ports on your system.

# -----  Handling dial-in or switched lines  -----
#   The first section changes the value of REMOTE based on the login name.
# This lets different users have different assumed terminals.  REMOTE is only
# used for switched or dial-in terminals.
#   The notation:
#	[ xxxxx ]  &&  yyyyy
#
# is equivalent to:
#	if [ xxxxx ]
#	then
#		yyyyy
#	fi
#
# but is shorter and probably faster.  Since you are only changing the value
# of REMOTE, it allows one entry per line, keeping this section compact.  You
# could also use a 'case' statement if you have lots of LOGNAME's, many with
# the same value of REMOTE.

#   The second section describes how ports are connected and used on your
# machine.  Direct-connects will typically have a literal value listed.
# Switched or dial-in lines will use the variable REMOTE, perhaps after it has
# been modified by the first section.
# If a range of ports has the same kind of terminal, then you can use the
# shell's wild-card abilities to advantage.  Remember that case matches are
# processed in order, so the first match is the one executed.


# 			-----  Examples -----

# --- Remote terminals:  through a switch, not a dial-in
#[ "$LOGNAME" = "gg" ]  &&  REMOTE=155		# GG's in-cube terminal


# --- Main port descriptions:
#	direct-connect ports are listed literally
#	switched ports use REMOTE variable
#
#case `tty` in
#	*console )	ASSUME=vt100    ;;	# direct-connect
#
#	*tty1[0-9] )	ASSUME=vt100    ;;	# direct ports
#	*tty2[0-9] )	ASSUME=$REMOTE  ;;	# switched ports
#
	# Any ports not matching one of the above patterns will result in
	# the original value of ASSUME being used.
#esac

unset REMOTE		# no longer needed, only ASSUME is used later


# This file is invoked with a '. TermAssume' inside 'TermIs', so the variables
# set here will be set upon return from this file.  This is logically
# equivalent to the C directive "#include".

