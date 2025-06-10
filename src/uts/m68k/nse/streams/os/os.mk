#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-port:nse/streams/os/os.mk	10.12"
#
#		Copyright 1984 AT&T
#

STRIP = strip
INCRT = ../../..
LIBNAME = ../../../cf/lib.streams
DASHG = 
DASHO = -O
CFLAGS= -I$(INCRT) -DINKERNEL $(DASHG) $(DASHO)
FRC =

FILES = \
	$(LIBNAME)(streamio.o) 

all:	pickup $(LIBNAME) 

.PRECIOUS:	$(LIBNAME)

pickup:
	-@if [ "`ls *.o 2>/dev/null`" ] ; \
	then \
		echo "\tar ruv $(LIBNAME) *.o" ; \
		ar ruv $(LIBNAME) *.o ; \
		echo "\trm -f *.o" ; \
		rm -f *.o  ; \
	fi

$(LIBNAME):	$(FILES)
	ar ruv $(LIBNAME) *.o
	rm -f *.o

.c.a:
	$(CC) $(CFLAGS) -c $<

clean:
	-rm -f *.o

clobber:	clean
	-rm -f $(LIBNAME)

FRC:

#
# Header dependencies
#

$(LIBNAME)(streamio.o): streamio.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/strstat.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/poll.h \
	$(INCRT)/sys/termio.h \
	$(INCRT)/sys/ttold.h \
	$(INCRT)/sys/systm.h \
	$(FRC)

