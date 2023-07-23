# Shell file that actually does the FULL BACKUP
# Must be SuperUser to invoke.
# required args:
#	1 = root-name
# archive description found in Archive


. BUsetup

[ $# -lt 1 ]  &&  FAIL "$0: No root-name provided"
DIR=$1


MESSAGE="'$DIR' not backed up."
.  ArchGet	# load my variables

# Should we skip the 'fsck' on this one?
INPUT=`YesNo  "Perform file-system sanity check on '$DIR'?" `

if [ "$INPUT" = "YES" ] ; then		# chose to check it...
	LABEL=`HeadFor $DIR`		# contains device-name
	[ "$LABEL" ]  ||  {
		echo >/dev/tty "\nNo device-name found for '$DIR'" \
			"\nNo action taken"
		exit 2
	}

	set -- $LABEL ; DEVICE=$3	# get basic device-name
	if  fexpr "$DEVICE" : '@.*'  >/dev/null
	then
		# Remote resource: can't fsck it
		echo >/dev/tty  \
			"\n'$DIR' is a remote resource.  No fsck done."
	else
		# Not remote resource, do fsck
		RAW_DEVICE="/dev/r$DEVICE"

		sync; sync; sync;		# flush outstanding writes

		# Put a # before the 'echo' to really do the 'fsck'
		# echo >/dev/tty \
		/etc/fsck -n "$RAW_DEVICE"  ||  {
			# fsck better not change anything
			echo >/dev/tty "\nInsane file-system" \
				"\n\t$DIR not backed up"
			exit 2
		}
	fi
else
	# Give warning that file-system wasn't sanity-checked
	echo >/dev/tty "\nWARNING:  no sanity check done on '$DIR'\n"
fi


ACTION="write"
.  Confirm

# may have changed as a result of 'fsck', or fsck may have been skipped
# so get fs-info again
LABEL=`HeadFor $DIR`		# label contains FS-rebuilding information

echo >>$FULL_LOG  "### Begin backup of $DIR\n"

# do the actual BACKUP, but it is ATTENDED
BACKUP  $DIR  /dev/null   $WRITE_VERIFY  -m  \
	-L "$LABEL"  $VERBOSE_FUL  -f $AR_NAME $AR_REST  . \
	| tee /dev/tty  >>$FULL_LOG

exit $?
# output appears on tty, and is appended to log file
