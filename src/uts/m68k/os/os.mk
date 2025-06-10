#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-port:os/os.mk	10.12"
#
#		Copyright 1984 AT&T
#

STRIP = strip
INCRT = ..
LIBNAME = ../cf/lib.os
DASHG = 
DASHO = -O
CFLAGS= -I$(INCRT) -DINKERNEL $(DASHG) $(DASHO)
FRC =

FILES = \
	$(LIBNAME)(acct.o) \
	$(LIBNAME)(bio.o) \
	$(LIBNAME)(clock.o) \
	$(LIBNAME)(disp.o) \
	$(LIBNAME)(exec.o) \
	$(LIBNAME)(exit.o) \
	$(LIBNAME)(fault.o) \
	$(LIBNAME)(fio.o) \
	$(LIBNAME)(flock.o) \
	$(LIBNAME)(fork.o) \
	$(LIBNAME)(fstyp.o) \
	$(LIBNAME)(getpages.o) \
	$(LIBNAME)(grow.o) \
	$(LIBNAME)(iget.o) \
	$(LIBNAME)(lock.o) \
	$(LIBNAME)(locking.o) \
	$(LIBNAME)(machdep.o) \
	$(LIBNAME)(main.o) \
	$(LIBNAME)(malloc.o) \
	$(LIBNAME)(move.o) \
	$(LIBNAME)(nami.o) \
	$(LIBNAME)(page.o) \
	$(LIBNAME)(physio.o) \
	$(LIBNAME)(pipe.o) \
	$(LIBNAME)(pmmu.o) \
	$(LIBNAME)(prf.o) \
	$(LIBNAME)(probe.o) \
	$(LIBNAME)(region.o) \
	$(LIBNAME)(sched.o) \
	$(LIBNAME)(sig.o) \
	$(LIBNAME)(slp.o) \
	$(LIBNAME)(space.o) \
	$(LIBNAME)(startup.o) \
	$(LIBNAME)(string.o) \
	$(LIBNAME)(subr.o) \
	$(LIBNAME)(swapalloc.o) \
	$(LIBNAME)(swtch.o) \
	$(LIBNAME)(sys2.o) \
	$(LIBNAME)(sys3.o) \
	$(LIBNAME)(sys4.o) \
	$(LIBNAME)(sysent.o) \
	$(LIBNAME)(sysm68k.o) \
	$(LIBNAME)(text.o) \
	$(LIBNAME)(trap.o) \
	$(LIBNAME)(utssys.o)

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

$(LIBNAME)(acct.o): acct.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/acct.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/conf.h \
	$(FRC)

$(LIBNAME)(bio.o): bio.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/fs/s5macros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/iobuf.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/cmn_err.h \
	$(FRC)

$(LIBNAME)(clock.o): clock.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/tuneable.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/callo.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/swap.h \
	$(FRC)

$(LIBNAME)(disp.o): disp.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/debug.h \
	$(FRC)

$(LIBNAME)(exec.o): exec.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/map.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/acct.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/reg.h \
	$(INCRT)/sys/tuneable.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/message.h \
	$(INCRT)/sys/conf.h \
	$(FRC)

$(LIBNAME)(exit.o): exit.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/debug.h \
	$(FRC)

$(LIBNAME)(fault.o): fault.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/pfdat.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/reg.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/getpages.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/conf.h \
	$(FRC)

$(LIBNAME)(fio.o): fio.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/nami.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/acct.h \
	$(INCRT)/sys/open.h \
	$(INCRT)/sys/cmn_err.h \
	$(FRC)

$(LIBNAME)(flock.o): flock.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/sema.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/fcntl.h \
	$(INCRT)/sys/flock.h \
	$(FRC)

$(LIBNAME)(fork.o): fork.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/pfdat.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/acct.h \
	$(INCRT)/sys/tuneable.h \
	$(FRC)

$(LIBNAME)(fstyp.o): fstyp.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/nami.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/fs/s5macros.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/conf.h \
	$(FRC)

$(LIBNAME)(getpages.o): getpages.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/tuneable.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/pfdat.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/getpages.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/cmn_err.h \
	$(FRC)

$(LIBNAME)(grow.o): grow.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/pfdat.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/tuneable.h \
	$(FRC)

$(LIBNAME)(iget.o): iget.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/ino.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/open.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/cmn_err.h \
	$(FRC)

$(LIBNAME)(lock.o): lock.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/lock.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/tuneable.h \
	$(FRC)

$(LIBNAME)(locking.o): locking.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/flock.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/unistd.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/cmn_err.h \
	$(FRC)

$(LIBNAME)(machdep.o): machdep.c \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/reg.h \
	$(FRC)

$(LIBNAME)(main.o): main.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/utsname.h \
	$(INCRT)/sys/cmn_err.h \
	$(FRC)

$(LIBNAME)(malloc.o): malloc.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/map.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/debug.h \
	$(FRC)

$(LIBNAME)(move.o): move.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/buf.h \
	$(FRC)

$(LIBNAME)(nami.o): nami.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/nami.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/conf.h \
	$(FRC)

$(LIBNAME)(page.o): page.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/map.h \
	$(INCRT)/sys/pfdat.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/swap.h \
	$(INCRT)/sys/tuneable.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/conf.h \
	$(FRC)

$(LIBNAME)(physio.o): physio.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/getpages.h \
	$(INCRT)/sys/swap.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/cmn_err.h \
	$(FRC)

$(LIBNAME)(pipe.o): pipe.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/conf.h \
	$(FRC)

$(LIBNAME)(pmmu.o): pmmu.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/pfdat.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/map.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/debug.h \
	$(FRC)

$(LIBNAME)(prf.o): prf.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/mvme131.h \
	$(INCRT)/sys/cmn_err.h \
	$(FRC)

$(LIBNAME)(probe.o): probe.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/pfdat.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/tuneable.h \
	$(INCRT)/sys/cmn_err.h \
	$(FRC)

$(LIBNAME)(region.o): region.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/pfdat.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/ipc.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/shm.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/tuneable.h \
	$(INCRT)/sys/message.h \
	$(INCRT)/sys/conf.h \
	$(FRC)

$(LIBNAME)(sched.o): sched.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/tuneable.h \
	$(INCRT)/sys/getpages.h \
	$(INCRT)/sys/debug.h \
	$(FRC)

$(LIBNAME)(sig.o): sig.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/pfdat.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/nami.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/reg.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/message.h \
	$(INCRT)/sys/conf.h \
	$(FRC)

$(LIBNAME)(slp.o): slp.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/debug.h \
	$(FRC)

$(LIBNAME)(space.o): space.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/acct.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/pfdat.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/swap.h \
	$(INCRT)/sys/mount.h \
	$(FRC)

$(LIBNAME)(startup.o): startup.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/pfdat.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/map.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/reg.h \
	$(INCRT)/sys/tty.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/cmn_err.h \
	$(FRC)

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

$(LIBNAME)(subr.o): subr.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/debug.h \
	$(FRC)

$(LIBNAME)(swapalloc.o): swapalloc.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/pfdat.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/swap.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/open.h \
	$(INCRT)/sys/getpages.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/tuneable.h \
	$(FRC)

$(LIBNAME)(swtch.o): swtch.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(FRC)

$(LIBNAME)(sys2.o): sys2.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/nami.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/fcntl.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/message.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/stropts.h \
	$(INCRT)/sys/stream.h \
	$(INCRT)/sys/poll.h \
	$(INCRT)/sys/conf.h \
	$(FRC)

$(LIBNAME)(sys3.o): sys3.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/iobuf.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/fcntl.h \
	$(INCRT)/sys/flock.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/stat.h \
	$(INCRT)/sys/swap.h \
	$(INCRT)/sys/statfs.h \
	$(INCRT)/sys/ioctl.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/ttold.h \
	$(INCRT)/sys/message.h \
	$(INCRT)/sys/utsname.h \
	$(FRC)

$(LIBNAME)(sys4.o): sys4.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/tuneable.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/nami.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/clock.h \
	$(INCRT)/sys/conf.h \
	$(FRC)

$(LIBNAME)(sysent.o): sysent.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/systm.h \
	$(FRC)

$(LIBNAME)(sysm68k.o): sysm68k.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/sysm68k.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/utsname.h \
	$(INCRT)/sys/swap.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/mvme131.h \
	$(INCRT)/sys/reg.h \
	$(FRC)

$(LIBNAME)(text.o): text.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/cmn_err.h \
	$(FRC)

$(LIBNAME)(trap.o): trap.c \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/reg.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/trap.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/utsname.h \
	$(INCRT)/sys/mvme131.h \
	$(INCRT)/sys/cmn_err.h \
	$(FRC)

$(LIBNAME)(utssys.o): utssys.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/uadmin.h \
	$(INCRT)/sys/mount.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/fstyp.h \
	$(INCRT)/sys/ustat.h \
	$(INCRT)/sys/statfs.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/utsname.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/conf.h \
	$(FRC)

$(LIBNAME)(string.o): string.c \
	$(INCRT)/sys/types.h \
	$(FRC)
