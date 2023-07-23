
# most of the extraneous stuff is here to support sysadm format
# in an attempt to leave that code relatively untouched

if [ $# = 0 ]
then
	echo "Syntax:  $0  <device>"
	exit 1
fi

verify=no

if [ ${1} = "-v" ]
then
	verify=yes
	shift
fi
rtn=`/etc/dinit -fob /stand/m68k/boots/vmeboot m320dsdd5 $1 2>&1`

if [ ${?} = 0 -a -z "${rtn}" ]
then
    if [ ${verify} = yes ]
    then
	echo "Verification in progress"
	dd <${1} >/dev/null 2>/dev/tty bs=32k
	exit ${?}
    else
        exit 0
    fi
else
	exit 1
fi
