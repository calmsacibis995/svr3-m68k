TMP=/tmp
append=NO

#Due to a recent change to dinit, the usefulness of badtrack as a
#conversion utility is severly reduced.  It is still handy,
#though as a front end to dinit, for storing the info in a file
#plus it ignores bad input whereas dinit doesn't, and allows for append.
#Thus, rather than trash it, we set an environment var so we could
#sometimes use it as a conversion tool again.  For now, though, it
#basically passes its arguments through

FORMAT=${FORMAT:-"HDCYL"}
Mfg=${Mfg:-"m"}

# we don't use getopt here, 'cause badtrack exists on the bootable
# floppy also.  Getopt is too expensive to have there.

case "$1" in
    -a)
	append=YES
	shift
	;;
    -*)
    	echo "Syntax:  $0  [-a]  <drive>"
    	echo "\twhere <drive> is:"
	echo "\tm320_[01], m320_1d[01], m360_[02], m360_1d[02]"
    	exit 1
    	;;
    *)
	;;
esac


case "$1" in
m320_*)
    case "$Mfg" in
    m|s)
	    numheads=8 ;;
    t)
	    numheads=10 ;;
    p)
	    numheads=7 ;;
    *)
	    numheads=8 ;;
    esac

    echo "\n\t[Winchester (${numheads} heads)]"
    ;;
m360_*)
    numheads=10
    echo "\n\t[SMD (10 heads)]"
    ;;
*)
    if [ ! -z "$1" ]
    then
	echo "\tUnknown drive $1\n"
    	echo "Syntax:  $0  [-a]  <drive>"
    	echo "\twhere <drive> is:"
	echo "\tm320_[01], m320_1d[01], m360_[02], m360_1d[02]"
	exit 1
    else
	numheads=8
	echo "\tAssuming Winchester (8 heads)"
    fi
    ;;
esac

outfile=/etc/badtracks/$1

trap 'rm -f ${TMP}/badtrack$$;[ ! -z "$1" ] && echo "*** No changes made to $outfile";exit 1' 1 2 15

>${TMP}/badtrack$$

echo "\nEnter defects in form:"
echo "\t<head> <cylinder> <Return>"
echo "End with <. Return> or <Ctrl-D>"

head=

until [ "$head" = "." ]
do
    echo "> \c"
    read head cyl

    [ "$head" = "." ] && continue

    if [ "$head" -ge $numheads ]
    then
	echo "\t$head:  Maximum head number is `expr $numheads - 1`"
	echo "\tcontinue"
	continue
    fi

    track=`expr \( $cyl \* $numheads \) + $head`
    if [ $? = 0 ]
    then
	if [ "$FORMAT" = HDCYL ]
	then
	    echo "\t\t$head\t$cyl"
	else
	    echo "\t\t$track"
	fi

	if [ ! -z "$1" ]
	then
	    if [ "$FORMAT" = HDCYL ]
	    then
		echo "$head	$cyl" >> ${TMP}/badtrack$$
	    else
	        echo $track >> ${TMP}/badtrack$$
	    fi
	fi
    else
    echo "\t\tcontinue"
    fi
done

if [ ! -z "$1" -a "$append" = "NO" ]
then
    mv ${TMP}/badtrack$$ ${outfile}
elif [ ! -z "$1" ]
then
    cat ${TMP}/badtrack$$ >> ${outfile}
fi

rm -f ${TMP}/badtrack$$
