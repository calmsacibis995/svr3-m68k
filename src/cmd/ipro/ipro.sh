#######################################################################
#######################################################################
####                                                               ####
####                                                               ####
####  (C) Copyright  Motorola, Inc.  1985, 1986, 1987              ####
####  All Rights Reserved                                          ####
####  Motorola Confidential/Proprietary                            ####
####                                                               ####
####                                                               ####
####  SOFTWARE PRODUCT:  IPRO                                      ####
####                                                               ####
####                                                               ####
####  PROGRAM:  ipro.sh - runs ipro and verifies correctness of    ####
####                      reorganizations.                         ####
####                                                               ####
####                                                               ####
####  AUTHOR:  Norman L. Nealy                                     ####
####           Senior Software Development Engineer                ####
####           Motorola Microcomputer Division                     ####
####           3013 South 52nd Street  MS-J3                       ####
####           Tempe, Arizona  85282                               ####
####           (602) 438-5724/5600                                 ####
####                                                               ####
####                                                               ####
####  ORIGINAL DATE:  November 1, 1985                             ####
####                                                               ####
####                                                               ####
####  SOURCE LOCATION:  /usr/src/cmd/ipro/ipro.sh                  ####
####                                                               ####
####                                                               ####
#######################################################################
#######################################################################


VERBOSE="OFF"
CMDLINE="OFF"
CLFILSYS="OFF"
CLFILNAM="OFF"
STDINPUT="OFF"
SIFILSYS="OFF"
SIFILNAM="OFF"
DELTAINOBLKS="OFF"
DELTAFREEBLKS="OFF"
LOGFILE="OFF"

vFLG="v"
cFLG="c"
CFLG="C"
sFLG="s"
SFLG="S"
iFLG="i"
fFLG="f"
lFLG="l"

IPROFLAGS=""

MNT="/mnt"

FSLIST=""

TMP="/usr/tmp"

LS1FILE="$TMP/ls1file"
LS2FILE="$TMP/ls2file"

CRC1FILE="$TMP/crc1file"
CRC2FILE="$TMP/crc2file"

LSDIFF="$TMP/lsdiff"
CRCDIFF="$TMP/crcdiff"

LSCRCFILES="$LS1FILE $LS2FILE $CRC1FILE $CRC2FILE $LSDIFF $CRCDIFF"

CWD="`pwd`"


set -- `getopt "$vFLG$cFLG$CFLG$sFLG$SFLG$iFLG:$fFLG:$lFLG:" $*`

for CLA

   in $*
do
   case "$CLA" in

      -$vFLG )

         if [ "$VERBOSE" != "OFF" ]
         then
            echo "\n$0: ref. $CLA option: multiple $CLA options found."
            exit 1

         else

            IPROFLAGS="$IPROFLAGS $CLA"
            VERBOSE="ON"
            shift 1
         fi
         ;;

      -$cFLG )

         if [ "$CLFILSYS" != "OFF" ]
         then
            echo "\n$0: ref. $CLA option: multiple $CLA options found."
            exit 2

         elif [ "$CLFILNAM" != "OFF" ]
         then
            echo "\n$0: ref. $CLA option:\c"
            echo " $CLA or -$CFLG allowed but not both."
            exit 3

         else

            CMDLINE="ON"
            CLFILSYS="ON"
            shift 1
         fi
         ;;

      -$CFLG )

         if [ "$CLFILNAM" != "OFF" ]
         then
            echo "\n$0: ref. $CLA option: multiple $CLA options found."
            exit 4

         elif [ "$CLFILNAM" != "OFF" ]
         then
            echo "\n$0: ref. $CLA option:\c"
            echo " $CLA or -$cFLG allowed but not both."
            exit 5

         else

            CMDLINE="ON"
            CLFILNAM="ON"
            shift 1
         fi
         ;;

      -$sFLG )

         if [ "$SIFILSYS" != "OFF" ]
         then
            echo "\n$0: ref. $CLA option: multiple $CLA options found."
            exit 6

         elif [ "$SIFILNAM" != "OFF" ]
         then
            echo "\n$0: ref. $CLA option:\c"
            echo " $CLA or -$SFLG allowed but not both."
            exit 7

         else

            STDINPUT="ON"
            SIFILSYS="ON"
            shift 1
         fi
         ;;

      -$SFLG )

         if [ "$SIFILNAM" != "OFF" ]
         then
            echo "\n$0: ref. $CLA option: multiple $CLA options found."
            exit 8

         elif [ "$SIFILNAM" != "OFF" ]
         then
            echo "\n$0: ref. $CLA option:\c"
            echo " $CLA or -$sFLG allowed but not both."
            exit 9

         else

            STDINPUT="ON"
            SIFILNAM="ON"
            shift 1
         fi
         ;;

      -$iFLG )

         NDIB="`expr 0 + $2  2> /dev/null`"

         if [ "$DELTAINOBLKS" != "OFF" ]
         then
            echo "\n$0: ref. $CLA option: multiple $CLA options found."
            exit 10

         elif [ -z "$2"  -o  -z "$NDIB"  -o  "$2" -ne "$NDIB" ]
         then
            echo "\n$0: ref. $CLA option:\c"
            echo " delta_inode_blocks $2 not numeric."
            exit 11

         else

            IPROFLAGS="$IPROFLAGS $CLA $2"
            DELTAINOBLKS="ON"
            shift 2
         fi
         ;;

      -$fFLG )

         NDFB="`expr 0 + $2  2> /dev/null`"

         if [ "$DELTAFREEBLKS" != "OFF" ]
         then
            echo "\n$0: ref. $CLA option: multiple $CLA options found."
            exit 12

         elif [ -z "$2"  -o  -z "$NDFB"  -o  "$2" -ne "$NDFB" ]
         then
            echo "\n$0: ref. $CLA option:\c"
            echo " delta_free_blocks $2 not numeric."
            exit 13

         else

            IPROFLAGS="$IPROFLAGS $CLA $2"
            DELTAFREEBLKS="ON"
            shift 2
         fi
         ;;

      -$lFLG )

         if [ "$LOGFILE" != "OFF" ]
         then
            echo "\n$0: ref. $CLA option: multiple $CLA options found."
            exit 14

         elif [ -z "$2" ]
         then
            echo "\n$0: ref. $CLA option:\c"
            echo " log_file_name must not be null."
            exit 15

         else

            IPROFLAGS="$IPROFLAGS $CLA $2"
            LOGFILE="ON"
            shift 2
         fi
         ;;

      -- )

         shift 1
         break 1
         ;;

      * )

         echo "\n$0: option $CLA violates command line specification."
         exit 16
         ;;
   esac
done


if [ "$CMDLINE" = "OFF"  -a  "$STDINPUT" = "OFF" ]
then
   CMDLINE="ON"
   CLFILSYS="ON"
fi


if [ "$CMDLINE" = "ON" ]
then
   if [ "$CLFILSYS" = "ON" ]
   then
      FSLIST="$FSLIST $*"

   elif [ "$CLFILNAM" = "ON" ]
   then
      for FSN

         in $*
      do
         FSLIST="$FSLIST `cat $FSN`"
      done

   else

      echo "\n$0: no operation specified for command line."
      exit 17
   fi
fi


if [ "$STDINPUT" = "ON" ]
then
   if [ "$SIFILSYS" = "ON" ]
   then
      while
         read FS
      do
         FSLIST="$FSLIST $FS"
      done

   elif [ "$SIFILNAM" = "ON" ]
   then
      while
         read FSN
      do
         FSLIST="$FSLIST `cat $FSN`"
      done

   else

      echo "\n$0: no operation specified for standard input."
      exit 18
   fi
fi


for FS

   in $FSLIST
do
   if [ ! -b $FS  -a  ! -c $FS ]
   then
      echo "\n$0: $FS: not a device file."
      rm -f $LSCRCFILES
      exit 19
   fi

   fsck $FS

   if [ "$?" -ne "0" ]
   then
      echo "\n$0: $FS: first fsck failed."
      rm -f $LSCRCFILES
      exit 20
   fi

   lscrc $FS $MNT $LS1FILE $CRC1FILE

   if [ "$?" -ne "0" ]
   then
      echo "\n$0: $FS: first lscrc failed."
      rm -f $LSCRCFILES
      exit 21
   fi

   ipro $IPROFLAGS $FS

   if [ "$?" -ne "0" ]
   then
      echo "\n$0: $FS: ipro failed."
      rm -f $LSCRCFILES
      exit 22
   fi

   fsck $FS

   if [ "$?" -ne "0" ]
   then
      echo "\n$0: $FS: first fsck failed."
      rm -f $LSCRCFILES
      exit 23
   fi

   lscrc $FS $LS2FILE $CRC2FILE

   if [ "$?" -ne "0" ]
   then
      echo "\n$0: $FS: second lscrc failed."
      rm -f $LSCRCFILES
      exit 24
   fi

   diff $LS1FILE $LS2FILE | tee $LSDIFF

   diff $CRC1FILE $CRC2FILE | tee $CRCDIFF

   if [ "`cat $LSDIFF $CRCDIFF | wc -l`" -ne "0" ]
   then
      echo "\n$0: $FS: ipro verification failed."
      rm -f $LSCRCFILES
      exit 25

   else

      echo "\n$0: $FS: ipro verification passed."
      rm -f $LSCRCFILES
   fi
done

rm -f $LSCRCFILES

exit 0
