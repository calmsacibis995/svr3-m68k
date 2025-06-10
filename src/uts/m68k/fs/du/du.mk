#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-port:fs/du/du.mk	10.1"
ROOT = 
STRIP = strip
INCRT = ../..
LIBNAME = ../../cf/lib.fs
CC=cc

DASHG = 
DASHO = -O
DUDEBUG = YES
PFLAGS = $(DASHG) -DINKERNEL $(MORECPP)
CFLAGS= $(DASHO) $(PFLAGS) -DDUDEBUG=$(DUDEBUG)
DEFLIST=
FRC =

FILES = \
	$(LIBNAME)(duiget.o) \
	$(LIBNAME)(dulocking.o) \
	$(LIBNAME)(durdwri.o) \
	$(LIBNAME)(dusubr.o) \
	$(LIBNAME)(dusys3.o)

all:	pickup $(LIBNAME)

.PRECIOUS: $(LIBNAME)

pickup:
	-@if [ "`ls *.o 2>/dev/null`" ] ; \
	then \
		echo "\tar ruv $(LIBNAME) *.o" ; \
		ar ruv $(LIBNAME) *.o ; \
		echo "\trm -f *.o" ; \
		rm -f *.o  ; \
	fi

$(LIBNAME): $(FILES)
	ar ruv $(LIBNAME) *.o
	rm -f *.o

.c.a:
	$(CC) $(DEFLIST) -I$(INCRT) $(CFLAGS) -c $<

clean:
	-rm -f *.o

clobber:	clean
	-rm -f $(LIBNAME)

#
# Header dependencies
#

$(LIBNAME)(duiget.o): duiget.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/fs/s5inode.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/ino.h \
	$(INCRT)/sys/fs/s5filsys.h \
	$(INCRT)/sys/stat.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/rdebug.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/message.h \
	$(FRC)

$(LIBNAME)(dulocking.o): dulocking.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/nami.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/rdebug.h \
	$(INCRT)/sys/message.h \
	$(FRC)

$(LIBNAME)(durdwri.o): durdwri.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/fs/s5inode.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/rdebug.h \
	$(INCRT)/sys/message.h \
	$(INCRT)/sys/sysinfo.h \
	$(FRC)

$(LIBNAME)(dusubr.o): dusubr.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sema.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/comm.h \
	$(INCRT)/sys/nserve.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/fs/s5inode.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/fs/s5filsys.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/statfs.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/open.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/rdebug.h \
	$(INCRT)/sys/message.h \
	$(FRC)

$(LIBNAME)(dusys3.o): dusys3.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/fs/s5inode.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/nami.h \
	$(INCRT)/sys/fs/s5filsys.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/rdebug.h \
	$(INCRT)/sys/message.h \
	$(FRC)
