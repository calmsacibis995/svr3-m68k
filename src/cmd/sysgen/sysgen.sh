# sysgen.sh
#

OBJECTDIR=/usr/src/uts/m68k/cf
OBJECT=$OBJECTDIR/sysV68
TARGET=/stand/sysV68

SG_LIB=/usr/src/uts/m68k/sysgen
export SG_LIB

if [ "`id | grep root" = "" ]
then
	echo >&2 "$0: Permission denied."
	exit 1
fi

UsageError=false

EditSelectConfiguration=true
Interactive=true

GenerateBuildInstall=false
EditDescriptions=false
EnableDisableChange=false

while [ $# -ne 0 ]
do
	case "$1" in
	-l )
		if [ $# -ge 2 ]
		then
			SG_LIB=$2
			shift
		else
			UsageError=true
			break
		fi
		;;
	-d )
		if [ $EditSelectConfiguration = true ]
		then
			EditDescriptions=true
			EditSelectConfiguration=false
			SG_OPTIONS=$1
		else
			UsageError=true
			break
		fi
		;;
	-s | -D | -c )
		if [ \( $EditSelectConfiguration = true -o \
		     $EnableDisableChange = true \) -a \
		     $# -ge 2 ]
		then
			EnableDisableChange=true
			EditSelectConfiguration=false
			SG_OPTIONS="$SG_OPTIONS $1 '$2'"
			Interactive=false
			shift
		else
			UsageError=true
			break
		fi
		;;
	-g | -b | -i | -gb | -bg | -gi | -ig | -bi | \
	-ib | -gbi | -gib | -bgi | -big | -igb | -ibg )
		if [ $EditSelectConfiguration = true -o \
		     $GenerateBuildInstall = true ]
		then
			GenerateBuildInstall=true
			EditSelectConfiguration=false
			SG_OPTIONS="$SG_OPTIONS $1"
			Interactive=false
		else
			UsageError=true
			break
		fi
		;;
	-- )
		break
		;;
	-* )
		UsageError=true
		break
		;;
	* )
		break
		;;
	esac
	shift
done

if [ $UsageError = true ]
then
	echo >&2 "Usage:\n\
\t$0 [-l path] [configuration]\n\
\t$0 [-l path] -d [collection]\n\
\t$0 [-l path] [-gbi] [configuration]\n\
\t$0 [-l path] [-s/D collection]... [-c comment] [configuration]...\n"
	exit 2
fi

if [ $Interactive = true ]
then
	echo "$0: Loading configuration information from $SG_LIB."
fi

if [ $EditSelectConfiguration = true -o $EnableDisableChange = true ]
then
	cd $SG_LIB/machines
	set -- $*
elif [ $EditDescriptions = true ]
then
	cd $SG_LIB/descriptions
	set -- $*
fi

eval $SG_LIB/sysgen -l $SG_LIB $SG_OPTIONS $*
if [ $? -ne 0 ]
then
	exit 1
fi

while read line
do
	set -- $line

	case $1 in
	"Configuration:" )
		continue
		;;

	"Rebuild:" )
		if [ $2 != true ]
		then
			continue
		fi

		if [ $Interactive = true ]
		then
			echo \
	"$0: Rebuilding the operating system in $OBJECTDIR."
		fi

		cd $OBJECTDIR
		for file in master dfile
		do
			rm -f $OBJECTDIR/$file
			cp $SG_LIB/$file $OBJECTDIR
			chmod -w $OBJECTDIR/$file
		done

		if [ $Interactive = true ]
		then
			read master_line <$OBJECTDIR/master
			x=`expr "$master_line" : '.*: \(.*\)\.'`
			echo "$0: using files based on configuration: $x."
		fi

		make 2>&1 | tee $SG_LIB/make.log
		;;

	"Install:" )
		if [ $2 != true ]
		then
			continue
		fi

		if [ -r $OBJECT -a -x $OBJECT ]
		then
			if [ $Interactive = true ]
			then
				echo \
		"$0: Installing the operating system in $TARGET."
			fi

			mv $TARGET $TARGET.save
			if cp $OBJECT $TARGET
			then
				rm -f $TARGET.save
			else
				mv $TARGET.save $TARGET
			fi
		else
			echo >&2 "$0: $OBJECT is not executable."
			exit 1
		fi
		;;
	esac
done <$SG_LIB/sgdata
