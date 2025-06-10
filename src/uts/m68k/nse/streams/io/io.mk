#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-port:nse/streams/io/io.mk	10.2"
ROOT =
INCRT = ../../..
LIBNAME = ../../../cf/lib.streams

DASHG =
DASHO = -O
PFLAGS = $(DASHG) -I$(INCRT) -DINKERNEL 
CFLAGS = $(DASHO) $(PFLAGS)
FRC =

DFILES = \
	$(LIBNAME)(clone.o) \
	$(LIBNAME)(log.o) \
	$(LIBNAME)(sp.o) \
	$(LIBNAME)(stream.o) \
	$(LIBNAME)(timod.o) \
	$(LIBNAME)(tirdwr.o) 

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

$(LIBNAME):	$(DFILES)
	ar ruv $(LIBNAME) *.o
	rm -f *.o

.c.a:
	$(CC) $(CFLAGS) -c $<

clean:
	-rm -f *.o

clobber:
	-rm -f *.o
	-rm -f $(LIBNAME)

FRC:

#
# Header dependencies
#

$(LIBNAME)(clone.o): clone.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/debug.h \
	$(FRC)

$(LIBNAME)(log.o): log.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/strstat.h \
	$(INCRT)/sys/log.h \
	$(INCRT)/sys/strlog.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/debug.h \
	$(FRC)

$(LIBNAME)(sp.o): sp.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/stream.h \
	$(FRC)

$(LIBNAME)(stream.o): stream.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/strstat.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/tuneable.h \
	$(INCRT)/sys/map.h \
	$(FRC)

$(LIBNAME)(stubs.o): stubs.c \
	$(FRC)

$(LIBNAME)(timod.o): timod.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/tihdr.h \
	$(INCRT)/sys/timod.h \
	$(INCRT)/sys/tiuser.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(FRC)

$(LIBNAME)(tirdwr.o): tirdwr.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/tihdr.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/errno.h \
	$(FRC)
