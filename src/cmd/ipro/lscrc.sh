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
####  PROGRAM:  lscrc.sh - perform an ls and a crc on all files    ####
####                       found in the specified filesystem.      ####
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
####  SOURCE LOCATION:  /usr/src/cmd/ipro/lscrc.sh                 ####
####                                                               ####
####                                                               ####
#######################################################################
#######################################################################


CAC="4"

if [ "$#" -ne "$CAC" ]
then
   echo "\n$0: usage: $0 <filesystem_name> <mount_point> <ls_file>\c"
   echo " <crc_file>"
   exit 1

elif [ -z "$1"  -o  -z "$2"  -o  -z "$3"  -o  -z "$4" ]
then
   echo "\n$0: null arguments."
   exit 2
fi

CWD="`pwd`"

> "$3"
> "$4"

if [ -c "$1" ]
then
   MNT=`echo $1 | sed -e 's/\/r/\//p'`

elif [ -b "$1" ]
then
   MNT="$1"

else
   echo "\n$0: non-device file."
   exit 3
fi

mount "$MNT" "$2"

cd "$2"

find . -print | sort -t"/" | while

   read FILE
do
   ls -lad "$FILE"  >> "$3"

   if [ -f "$FILE" ]
   then
      crc "$FILE"  >> "$4"
   else
      echo "NO CRC FOR $FILE"  >> "$4"
   fi
done

cd "$CWD"

umount "$MNT"

exit 0
