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
####  FILE:  Makefile - IPRO Make File.                            ####
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
####  SOURCE LOCATION:  /usr/src/cmd/ipro/ipro.mk                  ####
####                                                               ####
####                                                               ####
#######################################################################
#######################################################################


HEADERS=	/usr/include/stdio.h\
		/usr/include/ctype.h\
		/usr/include/fcntl.h\
		/usr/include/math.h\
		/usr/include/memory.h\
		/usr/include/string.h\
		/usr/include/time.h\
		/usr/include/sys/types.h\
		/usr/include/sys/fs/s5param.h\
		/usr/include/sys/fs/s5dir.h\
		/usr/include/sys/fs/s5fblk.h\
		/usr/include/sys/fs/s5filsys.h\
		/usr/include/sys/ino.h\
		/usr/include/sys/fs/s5inode.h\
		/usr/include/sys/signal.h\
		/usr/include/sys/stat.h\
		iprodefs.h

SOURCES=	main.c\
		blkreorder.c\
		cleanup.c\
		compltimes.c\
		dircmp.c\
		dircompact.c\
		fsblockio.c\
		getblkno.c\
		initblocks.c\
		inodecmp.c\
		inoreorder.c\
		isblkspec.c\
		ischrspec.c\
		isdev.c\
		isdevice.c\
		isdir.c\
		isfilsys.c\
		israw.c\
		isreg.c\
		loadblktbl.c\
		loadfrelst.c\
		loadinotbl.c\
		loadsupblk.c\
		longpow.c\
		memops.c\
		prcenter.c\
		printmesg.c\
		printtime.c\
		procblock.c\
		procfilsys.c\
		procinode.c\
		proclevel.c\
		sighand.c\
		terminate.c

OBJECTS=	main.o\
		blkreorder.o\
		cleanup.o\
		compltimes.o\
		dircmp.o\
		dircompact.o\
		fsblockio.o\
		getblkno.o\
		initblocks.o\
		inodecmp.o\
		inoreorder.o\
		isblkspec.o\
		ischrspec.o\
		isdev.o\
		isdevice.o\
		isdir.o\
		isfilsys.o\
		israw.o\
		isreg.o\
		loadblktbl.o\
		loadfrelst.o\
		loadinotbl.o\
		loadsupblk.o\
		longpow.o\
		memops.o\
		prcenter.o\
		printmesg.o\
		printtime.o\
		procblock.o\
		procfilsys.o\
		procinode.o\
		proclevel.o\
		sighand.o\
		terminate.o

all:		ipro		print

install:	ipro
		install -n /etc -i -m 755 -u root -g sys ipro /etc

headers:	$(HEADERS)

sources:	$(SOURCES)

objects:	$(OBJECTS)

clobber:	clean

clean:
		rm -f *.o .[a-z]* sighand* ipro

ipro:		$(OBJECTS)
		cc -o ipro $(OBJECTS) -lm
		chmod 755 ipro

$(OBJECTS):	$(HEADERS)

ipro.sh lscrc.sh:	s.$$@
			get -s -p $?  > $@
			chmod 755 $@

.iproxref:	$(SOURCES)
		cxref -c -w71 $(SOURCES)  > .iproxref

print:		$(PRFILES)
		$(HOME)/bin/prlp $?
		touch print
