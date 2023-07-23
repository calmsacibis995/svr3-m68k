# Shell file that actually does the incremental backup
# Must be SuperUser to invoke.
# required args:
#	1 = base dir
# optional args:
#	2.. = places to visit, and bru flags
#  the default archive is found in:  $ARCHIVE
#  the default since is in the file:   $LAST_FULL

. BUsetup

[ "$1" ]  ||  FAIL "$0: Need directory to backup from"
DIR="$1"; shift
PLACES="${*:-'.'}"	# by default, visit DIR
[ -r $TEST_FLAG ]  &&  { echo "PLACES =" $PLACES; }


# get the 'since' date
unset MESSAGE		# no message about "since"
.  SinceFull		# take since last full backup


MESSAGE="'$DIR' not backed up."
.  ArchGet	# load my variables

ACTION="write"
.  Confirm

# Write archive to device
# Prepare archive label for incremental backup
LABEL="~I $DIR ($PLACES) Incr since $SINCE"

>$INCR_LOG	# clear previous log file

# save output of BACKUP
BACKUP  $DIR  $LAST_INCR  $WRITE_VERIFY \
	-L "$LABEL"  -l  -n $SINCE  $VERBOSE_INC  \
	-f $AR_NAME $AR_REST $PLACES  2>&1  \
	| tee /dev/tty  >>$INCR_LOG

# output appears on tty and is appended to log file
