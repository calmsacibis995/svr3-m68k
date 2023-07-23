# Shell file determines correct value of TERM variable
# and sends it to standard output.
#   Copyright (c) 1986 by Motorola, Inc.
#
# Invoke as:
#	TERM=`TermIs`
# to determine the value of the TERM environment variable.
#
# To ask the user to select a terminal, invoke as:
#	TERM=`TermIs ask`
# and a short menu will be displayed and a line read from the terminal.

# The TERM variable should already be set with a default value if desired.
# If $L0 is non-null, then use it as the default TERM value.  
# See the model "profile" (/etc/stdprofile) for an example.
#    If the default TERM value is not found, then the user will be asked
# to choose a terminal.  This helps recover from typing mistakes made
# at login.  If any asking is done, the user will always be asked a second
# time if the first entry does not have a description.  There will be no
# more than two chances given for a correct terminal choice, so you better
# get it right by the second time around.


. TermAssume	# set value of $ASSUME

ASSUME=${ASSUME:-dumb}	# if empty, ASSUME="dumb"
TERM=${TERM:-$ASSUME}	# make assumption if no other TERM
PLACE=${TERMINFO:-/usr/lib/terminfo}	# look here for description


# Define local function that reads keyboard input
ReadTerm ()  {
	echo >/dev/tty	"$*" \
		"\n  Enter blank line to keep '$TERM', OR" \
		"\n  Enter a complete new name, OR" \
		"\n  Choose one of the following codes..." \
		"\n	0 = Motorola TM220" \
		"\n	8 = Motorola TM228" \
		"\n	9 = Motorola TM229" \
		"\n	f = Motorola FT45" \
		"\n	v = VT100 or compatible" \
		"\n	x = EXORterm 155" \
		"\n	d = dumb terminal" \
		"\nYour choice? \c"
	read </dev/tty  VALUE JUNK	# only use first word on line

	# if any VALUE entered, use it appropriately
	[ -n "$VALUE" ]  &&  {
		case "$VALUE" in
			[0Oo] )	TERM=tm220	;;	# knows letter "oh"
			[8]  )	TERM=tm228	;;
			[9]  )	TERM=tm229	;;
			[f]  )  TERM=ft45	;;
			[Vv] )	TERM=vt100	;;
			[Xx] )	TERM=155	;;
			[Dd] )	TERM=dumb	;;
			* )	TERM="$VALUE"	;;
		esac
	}
	# TERM holds resulting value
}


LookAsk () {
	# Checks if description exists, asks if no description
	SUB=`expr $TERM : '\(.\)'`		# first char of TERM name

	[ $0 = "-su" -o  -f $PLACE/$SUB/$TERM  -o  -f /usr/lib/terminfo/$SUB/$TERM  ]  ||  {
		# No description for this terminal...
		# Offer a chance to change it
		ReadTerm \
			"\n#### No description found for a '$TERM' terminal" \
			"\n$* or confirm this choice."
	}
	# value of TERM now reflects new or confirmed choice
}

if  [ "$1" = ask ]
then
	ReadTerm  "\nDefault terminal name:	$TERM"
elif [ -d ${PLACE} ]
then
	LookAsk "  Change"	# if not ask, just check for bad name
fi

if [ -d ${PLACE} ] 
then
LookAsk "#### LAST CHANCE to change"	# only asks again if unknown name
fi


echo $TERM	# send to waiting process
