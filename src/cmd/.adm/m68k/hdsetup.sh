FMT=/etc/dinit

verify() {
    vrfy=
    while [ "$vrfy" != y -a "$vrfy" != n ]
    do
	echo ">>> Is this really what you wanted? <y/n>: \c"
	read vrfy
    done
}

chkdsk() {
    echo
    check=
    while [ "$check" != y -a "$check" != n ]
    do
	echo ">>> Do you want to scan the disk for new bad tracks? <y/n>: \c"
	read check
    done

    if [ "$check" = y ]
    then
	FMODES="-c $FMODES"
    fi
}

getdevice() {

    echo

    DTYPE=
    TFILE=
    while [ -z "$DTYPE" ]
    do
	echo ">>> Enter the device <00,01,10,11,20,22,30,32>: \c"
	read device

	case $device in
	    00|01|10|11)
		    DTYPE=m32070m ;;
	    20|22|30|32)
		    DTYPE=m360337 ;;
	    *)
		    ;;
	esac
    done

    TFILE=/etc/badtracks/${device}
    FMTDEV=r${device}s7
}

getbads() {

    # If formatting or setting up config/bad tracks, and we don't yet
    # have a badtracks file to work from, get current bad
    # list from disk.  If format or config is blown, this may fail,
    # but it's worth a try.

    justmade=NO
    if [ ! -f "$TFILE" -a "$ans" != b ]
    then
	${FMT} -r $DTYPE /dev/$FMTDEV >$TFILE 
	if [ $? != 0 ]
	then
	    echo "\n\t*** Cannot get current bad track list from drive"
	    rm -f $TFILE
	    /etc/badtrack $device 
	    echo "\n*** Bad track list preserved in $TFILE"
	    justmade=YES
	else
	    echo "*** Current bad track list on disk preserved in $TFILE"
	fi
    fi

    if [ "$ans" != b -a "$justmade" = NO ]
    then
	echo
	add=
	while [ "$add" != y -a "$add" != n ]
	do
	    echo ">>> Do you wish to add to the bad track list in $TFILE? <y/n>: \c"
	    read add
	done

	if [ "$add" = y ]
	then
	    /etc/badtrack -a $device
	    echo "\n*** New bad tracks appended to  $TFILE"
	fi
    fi

}

echo "\n*** Hdsetup can format a 70Mb Winchester or 337Mb SMD,"
echo "*** initialize the disk-resident configuration parameters and"
echo "*** software/hardware bad track management facility"
echo "*** (redirection of defective tracks), install a new bootloader,"
echo "*** and scan the disk for bad spots. Formatting always includes bad"
echo "*** track initialization and installation of a bootloader."
echo "*** Bad track initialization always includes installation of"
echo "*** the bootloader (/stand/m68k/boots/vmeboot)."


while :
TFILE=
check=
do
    echo "\n*** Does a Hard Disk need to be formatted, initialized or made bootable?\n"
    ans=
    while [ "$ans" != f -a "$ans" != i -a "$ans" != b -a "$ans" != q ]
    do
	echo ">>> Enter  <f>ormat, <i>nit, <b>ootable, or <q>uit: \c"
	read ans
    done

    case "$ans" in
    f*)
	getdevice
	FMODES="-fob /stand/m68k/boots/vmeboot ${DTYPE} /dev/${FMTDEV}"
	getbads
	chkdsk
	echo "\n*** About to format   /dev/${FMTDEV}"
	;;
    i*)
	getdevice
	FMODES="-ob /stand/m68k/boots/vmeboot ${DTYPE} /dev/${FMTDEV}"
	getbads
	chkdsk
	echo "\n*** About to initialize configuration and bad track info on   /dev/${FMTDEV}"
	;;
    b*)
	getdevice
	echo "\n*** About to install  vmeboot  as bootloader on  /dev/${FMTDEV}"
	FMODES="-b /stand/m68k/boots/vmeboot ${DTYPE} /dev/${FMTDEV}"
	;;
    *)
	exit 1
	;;
    esac

    if [ "$check" = y ]
    then
	echo "*** The disk will be (DESTRUCTIVELY) scanned for new bad tracks"
    fi
    if [ -f "$TFILE" -a "$ans" != b ]
    then
	echo "*** with known bad track numbers taken from the file   $TFILE"
	FMODES="-t $TFILE $FMODES"
    fi
    verify
    if [ "$vrfy" = n ]
    then continue
    fi
    echo "\n*** ${FMT} $FMODES"
    ${FMT} $FMODES
    if [ $? != 0 ]
    then
	    echo "\n	**** ${FMT} failed ****"
    elif [ "$ans" != b ]
    then
	${FMT} -r ${DTYPE} /dev/${FMTDEV} > $TFILE 2>/dev/null
	echo "*** Current bad track list from disk $device  preserved in $TFILE"
    fi

done
