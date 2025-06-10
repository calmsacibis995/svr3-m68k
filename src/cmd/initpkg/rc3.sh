#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)initpkg:./rc3.sh	1.11"

if m68k
then echo "
#	\"Run Commands\" executed when the system is changing to init state 3,
#	same as state 2 (multi-user) but with remote file sharing.


. /etc/TIMEZONE

set \`who -r\`

if [ \$9 = \"S\" ]
then
	stty sane tab3 2>/dev/null

elif [ \$7 = \"3\" ]
then
	echo 'Changing to state 3.'
	if [ -d /etc/rc3.d ]
	then
		for f in /etc/rc3.d/K*
		{
			if [ -s \${f} ]
			then
				/bin/sh \${f} stop
			fi
		}
	fi
fi

if [ -d /etc/rc3.d ]
then
	for f in /etc/rc3.d/S*
	{
		if [ -s \${f} ]
		then
			/bin/sh \${f} start
		fi
	}
fi
if [ \$9 = \"S\" ]
then
	echo '
The system is ready.'
elif [ \$7 = \"3\" ]
then
	echo '
Change to state 3 completed.'
fi
echo 'Wait for RFS initialization message before using RFS commands.'
" >rc3
else
echo "
#	\"Run Commands\" executed when the system is changing to init state 3,
#	configuration is site dependant
" >rc3
fi
