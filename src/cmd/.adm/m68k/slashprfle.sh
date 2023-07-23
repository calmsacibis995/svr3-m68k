# This model profile is for "root" and has special things for setting
# up the terminal.  This is because the /usr file-system may not be mounted.
# This file IS NOT "/etc/profile", but is usually found in "/.profile".

PATH=${PATH}:/local/bin
# TERMCAP=
# TERMINFO=		# set to private values, if necessary

export PATH TERM TERMINFO TERMCAP CERASE CKILL CINTR CQUIT
export LOGNAME MAIL

CERASE="#"
CKILL="@"
CINTR="^?"

##############  Begin terminal setup section  ##############
#

# $L0 is the second word entered on the login-line when you login.
# For example:
#	login: user tm220
# would login as "user" and $L0 would be "tm220".
# If you enter nothing, then $L0 will be empty.

TERM=${L0:-$TERM}	# use $L0 value, if any, as default TERM
TERM=`TermIs`		# change to `TermIs ask` to ask for terminal
[ "$0" != "-su" ]  &&  TermSetup	# setup terminal only if multi-user
					# side effect: also affects 'su -'
. TermFuncs		# define function 'normal'...
normal			# then invoke it

#
##############  End terminal setup section  ##############

umask 000

FLOP=/dev/dsk/m320_2s0
OFLOP=/dev/dsk/m320_2s7
export FLOP OFLOP

# Local command aliases
bye () { exit; }
vis () { cat -v -t -e $*; }	# shows control characters as visible values
