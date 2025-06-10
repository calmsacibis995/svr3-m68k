#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-port:fs/s5/s5.mk	10.10"
ROOT = 
STRIP = strip
INCRT = ../..
LIBNAME = ../../cf/lib.s5
CC=cc

DASHG = 
DASHO = -O
PFLAGS = $(DASHG) -DINKERNEL
CFLAGS= $(DASHO) $(PFLAGS)
DEFLIST=
FRC =

FILES = \
	$(LIBNAME)(s5alloc.o) \
	$(LIBNAME)(s5blklist.o) \
	$(LIBNAME)(s5iget.o) \
	$(LIBNAME)(s5nami.o) \
	$(LIBNAME)(s5pipe.o) \
	$(LIBNAME)(s5rdwri.o) \
	$(LIBNAME)(s5subr.o) \
	$(LIBNAME)(s5sys2.o) \
	$(LIBNAME)(s5sys3.o)

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

$(LIBNAME):	$(FILES)
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

$(LIBNAME)(s5alloc.o): s5alloc.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/fs/s5macros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/fs/s5filsys.h \
	$(INCRT)/sys/fs/s5fblk.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/fs/s5inode.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/ino.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/conf.h \
	$(FRC)

$(LIBNAME)(s5blklist.o): s5blklist.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/fs/s5macros.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/fs/s5inode.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/conf.h \
	$(FRC)

$(LIBNAME)(s5iget.o): s5iget.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/fs/s5macros.h \
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
	$(INCRT)/sys/ino.h \
	$(INCRT)/sys/fs/s5filsys.h \
	$(INCRT)/sys/stat.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/cmn_err.h \
	$(FRC)

$(LIBNAME)(s5nami.o): s5nami.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/fs/s5macros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/fs/s5inode.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/nami.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/conf.h \
	$(FRC)

$(LIBNAME)(s5pipe.o): s5pipe.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/fs/s5macros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/fs/s5inode.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/cmn_err.h \
	$(FRC)

$(LIBNAME)(s5rdwri.o): s5rdwri.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/fs/s5macros.h \
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
	$(INCRT)/sys/fcntl.h \
	$(INCRT)/sys/flock.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/debug.h \
	$(FRC)

$(LIBNAME)(s5subr.o): s5subr.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/ustat.h \
	$(INCRT)/sys/fs/s5macros.h \
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
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/fcntl.h \
	$(INCRT)/sys/flock.h \
	$(FRC)

$(LIBNAME)(s5sys2.o): s5sys2.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/fs/s5macros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/dirent.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/nami.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/conf.h \
	$(FRC)

$(LIBNAME)(s5sys3.o): s5sys3.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/fs/s5macros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/ino.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/iobuf.h \
	$(INCRT)/sys/fs/s5filsys.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/fs/s5inode.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/fcntl.h \
	$(INCRT)/sys/flock.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/stat.h \
	$(INCRT)/sys/statfs.h \
	$(INCRT)/sys/ioctl.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/ttold.h \
	$(INCRT)/sys/open.h \
	$(INCRT)/sys/debug.h \
	$(FRC)
