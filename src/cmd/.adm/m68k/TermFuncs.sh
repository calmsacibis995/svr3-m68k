#  Terminal-related functions for shell use
#   Copyright (c) 1986 by Motorola, Inc.
#
# Invoke from within each user's .profile as follows:
#	. TermFuncs
# to define the functions 'normal' and 'gtty'.  Invoking 'normal' will
# set the tty modes and keys to sensible values for the current value of
# TERM, possibly modified by user preferences for erase/kill/intr/quit.

# # # # #  Begin Motorola-specific actions  # # # # #

gtty () { ( IFS=\; ; set `stty -a` ; echo "***\n$5 $6 $3 $4\n***" ) ; }

case "$TERM" in
	tm22[890]* )		# Motorola TM2201 and TM3000 series
		normal () {
		    stty -istrip tab3 \
			  intr ${CINTR:="^X"} \
		          erase ${CERASE:="^?"} \
			  kill ${CKILL:="^U"} \
			  quit ${CQUIT:="^|"}
		    ekey=

		    [ ${CERASE} = "^?" ] && ekey="'<X|' key"

		    echo "Setting erase to ${ekey:-${CERASE}}, \c"
		    echo "kill to ${CKILL}, intr to ${CINTR}"
		    echo '\033[?7h\c'	# enable auto-wrap
		}
		;;

	ft45* | tm40* | et40* )		# FT45 Fastrak terminals
		normal () {
		    stty  intr ${CINTR:="^X"} \
			  erase ${CERASE:="^H"} \
			  kill ${CKILL:="^U"} \
			  quit ${CQUIT:="^|"}
		    gtty
		}
		;;

	# Add your own terminals here...

	* )				# Anything not caught earlier....
		normal () {
		    stty  intr ${CINTR:="^?"} \
			  erase ${CERASE:="#"} \
			  kill ${CKILL:="@"} \
			  quit ${CQUIT:="^|"}
		     gtty
		}
		;;


esac
