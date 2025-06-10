#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-port:nudnix/nudnix.mk	10.15"
#
#		Copyright 1984 AT&T
#

ROOT = 
STRIP = strip
INCRT = ../..
LIBNAME = ../../cf/lib.du
MKBOOT = mkboot
DASHG = 
VER = mip
DASHO = -O
DUDEBUG = YES
CFLAGS= -I$(INCRT) -DINKERNEL $(MORECPP) $(DASHG) $(DASHO) -Dm68k -DDUDEBUG=$(DUDEBUG)
PFLAGS= -I$(INCRT) -DINKERNEL $(MORECPP) $(DASHG)
FRC =

FILES =\
	$(LIBNAME)(adv.o)\
	$(LIBNAME)(auth.o)\
	$(LIBNAME)(canon.o)\
	$(LIBNAME)(cirmgr.o)\
	$(LIBNAME)(comm.o)\
	$(LIBNAME)(fileop.o)\
	$(LIBNAME)(fumount.o)\
	$(LIBNAME)(netboot.o)\
	$(LIBNAME)(que.o)\
	$(LIBNAME)(queue.o)\
	$(LIBNAME)(rdebug.o)\
	$(LIBNAME)(recover.o)\
	$(LIBNAME)(remcall.o)\
	$(LIBNAME)(rfadmin.o)\
	$(LIBNAME)(rfcanon.o)\
	$(LIBNAME)(rfsys.o)\
	$(LIBNAME)(rmount.o)\
	$(LIBNAME)(rmove.o)\
	$(LIBNAME)(rnami.o)\
	$(LIBNAME)(rsc.o)\
	$(LIBNAME)(serve.o)

all: pickup $(LIBNAME)

pickup:
	-@if [ "`ls *.o 2>/dev/null`" ] ; \
	then \
		echo "\tar ruv $(LIBNAME) *.o" ; \
		ar ruv $(LIBNAME) *.o ; \
		echo "\trm -f *.o" ; \
		rm -f *.o  ; \
	fi

.PRECIOUS:	$(LIBNAME)

$(LIBNAME):	$(FILES)
	ar ruv $(LIBNAME) *.o
	rm -f *.o

.c.a:
	$(CC) $(CFLAGS) -c $<

clean:
	-rm -f *.o

clobber:	clean
	-rm -rf $(LIBNAME)

FRC:

#
# Header dependencies
#

$(LIBNAME)(adv.o): adv.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sema.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/comm.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/adv.h \
	$(INCRT)/sys/nserve.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/rdebug.h \
	$(FRC)

$(LIBNAME)(auth.o): auth.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sema.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/comm.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/nserve.h \
	$(INCRT)/sys/cirmgr.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/stat.h \
	$(INCRT)/sys/idtab.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/rdebug.h \
	$(FRC)

$(LIBNAME)(canon.o): canon.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(FRC)

$(LIBNAME)(cirmgr.o): cirmgr.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sema.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/comm.h \
	$(INCRT)/sys/recover.h \
	$(INCRT)/sys/rdebug.h \
	$(INCRT)/sys/nserve.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/cirmgr.h \
	$(INCRT)/sys/tihdr.h \
	$(INCRT)/sys/message.h \
	$(FRC)

$(LIBNAME)(comm.o): comm.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sema.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/message.h \
	$(INCRT)/sys/comm.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/nserve.h \
	$(INCRT)/sys/cirmgr.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/rdebug.h \
	$(FRC)

$(LIBNAME)(fileop.o): fileop.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sema.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/comm.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/nserve.h \
	$(INCRT)/sys/cirmgr.h \
	$(INCRT)/sys/message.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/stat.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/rdebug.h \
	$(FRC)

$(LIBNAME)(fumount.o): fumount.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sema.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/comm.h \
	$(INCRT)/sys/nserve.h \
	$(INCRT)/sys/cirmgr.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/message.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/adv.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/rdebug.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/recover.h \
	$(FRC)

$(LIBNAME)(netboot.o): netboot.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sema.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/comm.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/message.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/adv.h \
	$(INCRT)/sys/nserve.h \
	$(INCRT)/sys/recover.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/rdebug.h \
	$(FRC)

$(LIBNAME)(que.o): que.c \
	$(INCRT)/sys/que.h \
	$(FRC)

$(LIBNAME)(queue.o): queue.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sema.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/comm.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/message.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/recover.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/rdebug.h \
	$(FRC)

$(LIBNAME)(rdebug.o): rdebug.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sema.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/comm.h \
	$(INCRT)/sys/rdebug.h \
	$(FRC)

$(LIBNAME)(recover.o): recover.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sema.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/comm.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/message.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/nserve.h \
	$(INCRT)/sys/cirmgr.h \
	$(INCRT)/sys/recover.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/rfsys.h \
	$(INCRT)/sys/rdebug.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/fcntl.h \
	$(INCRT)/sys/flock.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(FRC)

$(LIBNAME)(remcall.o): remcall.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sema.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/comm.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/message.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/stat.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/idtab.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/nserve.h \
	$(INCRT)/sys/cirmgr.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/rdebug.h \
	$(INCRT)/sys/nami.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/cmn_err.h \
	$(FRC)

$(LIBNAME)(rfadmin.o): rfadmin.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sema.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/comm.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/message.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/nserve.h \
	$(INCRT)/sys/recover.h \
	$(INCRT)/sys/cirmgr.h \
	$(INCRT)/sys/adv.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/rdebug.h \
	$(INCRT)/sys/rfsys.h \
	$(INCRT)/sys/tihdr.h \
	$(FRC)

$(LIBNAME)(rfcanon.o): rfcanon.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sema.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/comm.h \
	$(INCRT)/sys/message.h \
	$(INCRT)/sys/dirent.h \
	$(INCRT)/sys/hetero.h \
	$(INCRT)/sys/rdebug.h \
	$(FRC)

$(LIBNAME)(rfsys.o): rfsys.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sema.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/comm.h \
	$(INCRT)/sys/nserve.h \
	$(INCRT)/sys/cirmgr.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/message.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/rdebug.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/recover.h \
	$(INCRT)/sys/rfsys.h \
	$(FRC)

$(LIBNAME)(rmount.o): rmount.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sema.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/comm.h \
	$(INCRT)/sys/nserve.h \
	$(INCRT)/sys/cirmgr.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/message.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/adv.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/rdebug.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/cmn_err.h \
	$(FRC)

$(LIBNAME)(rmove.o): rmove.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sema.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/comm.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/message.h \
	$(INCRT)/sys/rdebug.h \
	$(FRC)

$(LIBNAME)(rnami.o): rnami.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sema.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/comm.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/rdebug.h \
	$(INCRT)/sys/cmn_err.h \
	$(FRC)

$(LIBNAME)(rsc.o): rsc.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sema.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/comm.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/message.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/nserve.h \
	$(INCRT)/sys/cirmgr.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/rdebug.h \
	$(FRC)

$(LIBNAME)(serve.o): serve.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sema.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/fs/s5inode.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/comm.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/nserve.h \
	$(INCRT)/sys/cirmgr.h \
	$(INCRT)/sys/message.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/fs/s5filsys.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/fcntl.h \
	$(INCRT)/sys/nami.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/stat.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/idtab.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/rdebug.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/recover.h \
	$(FRC)

$(LIBNAME)(string.o): string.c \
	$(INCRT)/sys/types.h \
	$(FRC)
