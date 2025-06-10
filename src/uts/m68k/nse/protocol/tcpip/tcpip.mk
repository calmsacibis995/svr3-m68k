#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-port:nse/protocol/tcpip/tcpip.mk	10.2"
ROOT =
INCRT = ../../..
LIBNAME = ../../../cf/lib.tcpip

#	rfs debug flag - include tracker routine
#	remember to touch io/sitm.c and io/cmc/enp.c when changing flag
RFSDBUG = -DTRACK
DEBUG = -DDEBUG
DASHG =
DASHO = -O
PFLAGS = $(DASHG) -I$(INCRT) -DINKERNEL $(RFSDBUG)
CFLAGS = $(DASHO) $(PFLAGS) $(DEBUG)
FRC =

DFILES = \
	$(LIBNAME)(sitm.o)\
	$(LIBNAME)(tracker.o)

all:	pickup $(LIBNAME)
	-@cd cmc ; \
		echo "	$(MAKE) -f cmc.mk \"MAKE=$(MAKE)\" \"CC=$(CC)\" \"FRC=$(FRC)\" \"INCRT=$(INCRT)\" \"DASHO=$(DASHO)\" \"RFSDBUG=$(RFSDBUG)\" "; \
		$(MAKE) -f cmc.mk "MAKE=$(MAKE)" "CC=$(CC)" "FRC=$(FRC)" "INCRT=$(INCRT)" "DASHO=$(DASHO)" "RFSDBUG=$(RFSDBUG)"; 

.PRECIOUS:	$(LIBNAME)

pickup:
	-@if [ "`ls *.o 2>/dev/null`" ] ; \
	then \
		echo "\tar ruv $(LIBNAME) *.o" ; \
		ar ruv $(LIBNAME) *.o ; \
		echo "\trm -f *.o" ; \
		rm -f *.o  ; \
	fi

$(LIBNAME):	$(DFILES)
	ar ruv $(LIBNAME) *.o
	rm -f *.o

.c.a:
	$(CC) $(CFLAGS) -c $<

clean:
	-rm -f *.o
	-@cd cmc ; \
	echo "	"$(MAKE) -f cmc.mk clean ; \
	$(MAKE) -f cmc.mk clean

clobber:
	-rm -f *.o
	-rm -f $(LIBNAME)
	-@cd cmc ; \
	echo "	"$(MAKE) -f cmc.mk clobber ; \
	$(MAKE) -f cmc.mk clobber

FRC:

#
# Header dependencies
#
$(LIBNAME)(sitm.o): sitm.c \
	$(INCRT)/sys/CMC/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/tihdr.h \
	$(INCRT)/sys/tiuser.h \
	$(INCRT)/sys/CMC/signal.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/CMC/errno.h \
	$(INCRT)/sys/CMC/longnames.h \
	$(INCRT)/sys/CMC/netbfr.h \
	$(INCRT)/sys/CMC/ring.h \
	$(INCRT)/sys/CMC/CMC_proto.h \
	$(INCRT)/sys/CMC/CMC_socket.h \
	$(INCRT)/sys/CMC/in.h \
	$(INCRT)/sys/CMC/enp.h \
	$(INCRT)/sys/sitm.h \
	$(INCRT)/sys/log.h \
	$(INCRT)/sys/track.h \
	$(INCRT)/sys/cmn_err.h \
	$(FRC)

$(LIBNAME)(tracker.o): tracker.c \
	$(INCRT)/sys/track.h \
	$(FRC)
