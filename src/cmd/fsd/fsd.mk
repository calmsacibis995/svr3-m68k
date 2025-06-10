#ident	"@(#)fsd:fsd.mk	1.0"

# Makefile for Field Service Division utilities and files.


install:	load
	if grep service /etc/passwd >/dev/null ; then\
		:;\
	else\
		echo "Run ':mkcmd .adm' to create the 'service' login";\
		exit 1;\
	fi

	if [ ! -d /u/service ]; then\
		mkdir /u/service;\
		chown service /u/service;\
		chgrp  99 /u/service;\
	fi
	sh -x ./load 

clobber:

