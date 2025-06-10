#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-port:io/io.mk	10.2"
ROOT =
INCRT = ..
LIBNAME = ../cf/lib.io

DASHG =
DASHO = -O
PFLAGS = $(DASHG) -I$(INCRT) -DINKERNEL 
CFLAGS = $(DASHO) $(PFLAGS)
FRC =

DFILES = \
	$(LIBNAME)(badtrk.o) \
	$(LIBNAME)(chkparity.o) \
	$(LIBNAME)(clist.o) \
	$(LIBNAME)(clkZ8036.o) \
	$(LIBNAME)(cpvmeram.o) \
	$(LIBNAME)(dbuf.o) \
 	$(LIBNAME)(disjointio.o) \
	$(LIBNAME)(err.o) \
	$(LIBNAME)(errlog.o) \
	$(LIBNAME)(fpu881a.o) \
	$(LIBNAME)(gentty.o) \
	$(LIBNAME)(ipc.o) \
	$(LIBNAME)(lp050.o) \
	$(LIBNAME)(mc68B40.o) \
	$(LIBNAME)(mem.o) \
	$(LIBNAME)(mk68564.o) \
	$(LIBNAME)(msg.o) \
	$(LIBNAME)(mvme050.o) \
	$(LIBNAME)(mvme204.o) \
	$(LIBNAME)(mvme320.o) \
	$(LIBNAME)(mvme331.o) \
	$(LIBNAME)(mvme350.o) \
	$(LIBNAME)(mvme355.o) \
	$(LIBNAME)(mvme360.o) \
	$(LIBNAME)(osm.o) \
	$(LIBNAME)(partab.o) \
	$(LIBNAME)(physdsk.o) \
	$(LIBNAME)(prf.o) \
	$(LIBNAME)(r68560.o) \
	$(LIBNAME)(ram.o) \
	$(LIBNAME)(sem.o) \
	$(LIBNAME)(shm.o) \
	$(LIBNAME)(sxt.o) \
	$(LIBNAME)(tpoke00.o) \
	$(LIBNAME)(tt1.o) \
	$(LIBNAME)(tty.o) \
	$(LIBNAME)(isp.o) \
	$(LIBNAME)(ramdisk.o)

all:	subdirs pickup $(LIBNAME)

.PRECIOUS:	$(LIBNAME)

pickup:
	-@if [ "`ls *.o 2>/dev/null`" ] ; \
	then \
		echo "\tar ruv $(LIBNAME) *.o" ; \
		ar ruv $(LIBNAME) *.o ; \
		echo "\trm -f *.o" ; \
		rm -f *.o  ; \
	fi

subdirs:
	-@for i in * ; \
	do \
		if [ -d $$i -a -f $$i/$$i.mk ] ; then \
			cd $$i; \
			echo "	$(MAKE) -f $$i.mk \"MAKE=$(MAKE)\" \"CC=$(CC)\" \"FRC=$(FRC)\" \"INCRT=$(INCRT)\" \"DASHO=$(DASHO)\" "; \
			$(MAKE) -f $$i.mk "MAKE=$(MAKE)" "CC=$(CC)" "FRC=$(FRC)" "INCRT=$(INCRT)" "DASHO=$(DASHO)" ; \
			cd ..; \
		fi ; \
	done

$(LIBNAME):	$(DFILES)
	ar ruv $(LIBNAME) *.o
	rm -f *.o

.c.a:
	$(CC) $(CFLAGS) -c $<

clean:
	-@for i in * ; \
	do \
		if [ -d $$i -a -f $$i/$$i.mk ] ; then \
			cd $$i; \
			echo "	$(MAKE) -f $$i clean"; \
			$(MAKE) -f $$i.mk clean; \
			cd ..; \
		fi ; \
	done
	-rm -f *.o

clobber:
	-@for i in * ; \
	do \
		if [ -d $$i -a -f $$i/$$i.mk ] ; then \
			cd $$i; \
			echo "	$(MAKE) -f $$i clobber"; \
			$(MAKE) -f $$i.mk clobber; \
			cd ..; \
		fi ; \
	done
	-rm -f *.o
	-rm -f $(LIBNAME)

FRC:

#
# Header dependencies
#

$(LIBNAME)(badtrk.o): badtrk.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/dk.h \
	$(FRC)

$(LIBNAME)(chkparity.o): chkparity.c \
	$(INCRT)/sys/param.h \
	$(FRC)

$(LIBNAME)(clist.o): clist.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/tty.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/var.h \
	$(INCRT)/sys/cmn_err.h \
	$(FRC)

$(LIBNAME)(clkZ8036.o): clkZ8036.c \
	$(INCRT)/sys/types.h \
	$(FRC)

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

$(LIBNAME)(cpvmeram.o): cpvmeram.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/utsname.h \
	$(INCRT)/sys/elog.h \
	$(INCRT)/sys/erec.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/cpvmeram.h \
	$(FRC)

$(LIBNAME)(dbuf.o): dbuf.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/dbuf.h \
	$(INCRT)/sys/cmn_err.h \
	$(FRC)

$(LIBNAME)(disjointio.o): disjointio.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/iobuf.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/disjointio.h \
	$(INCRT)/sys/debug.h \
	$(FRC)

$(LIBNAME)(err.o): err.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/utsname.h \
	$(INCRT)/sys/elog.h \
	$(INCRT)/sys/erec.h \
	$(INCRT)/sys/sysmacros.h \
	$(FRC)

$(LIBNAME)(errlog.o): errlog.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/map.h \
	$(INCRT)/sys/utsname.h \
	$(INCRT)/sys/elog.h \
	$(INCRT)/sys/erec.h \
	$(INCRT)/sys/err.h \
	$(INCRT)/sys/iobuf.h \
	$(INCRT)/sys/var.h \
	$(FRC)

$(LIBNAME)(fpu881a.o): fpu881a.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/trap.h \
	$(FRC)

$(LIBNAME)(gentty.o): gentty.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/tty.h \
	$(INCRT)/sys/stream.h \
	$(FRC)

$(LIBNAME)(ipc.o): ipc.c \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/ipc.h \
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

$(LIBNAME)(lp050.o): lp050.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/tty.h \
	$(INCRT)/sys/lp0.h \
	$(INCRT)/sys/lprio.h \
	$(FRC)

$(LIBNAME)(mc68B40.o): mc68B40.c \
	$(INCRT)/sys/types.h \
	$(FRC)

$(LIBNAME)(mem.o): mem.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/immu.h \
	$(FRC)

$(LIBNAME)(mk68564.o): mk68564.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/tty.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/termio.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/mk68564.h \
	$(FRC)

$(LIBNAME)(msg.o): msg.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/map.h \
	$(INCRT)/sys/ipc.h \
	$(INCRT)/sys/msg.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/sysinfo.h \
	$(FRC)

$(LIBNAME)(mvme050.o): mvme050.c \
	$(INCRT)/sys/mc68153.h \
	$(FRC)

$(LIBNAME)(mvme204.o): mvme204.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/utsname.h \
	$(INCRT)/sys/elog.h \
	$(INCRT)/sys/erec.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/mvme204.h \
	$(FRC)

$(LIBNAME)(mvme320.o): mvme320.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/elog.h \
	$(INCRT)/sys/iobuf.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/dk.h \
	$(INCRT)/sys/mvme320.h \
	$(FRC)

$(LIBNAME)(mvme331.o): mvme331.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/tty.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/termio.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/mvme331.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(FRC)

$(LIBNAME)(mvme350.o): mvme350.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/elog.h \
	$(INCRT)/sys/iobuf.h \
	$(INCRT)/sys/mvme350.h \
	$(INCRT)/sys/dbuf.h \
	$(INCRT)/sys/cmn_err.h \
	$(FRC)

$(LIBNAME)(mvme355.o): mvme355.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/elog.h \
	$(INCRT)/sys/iobuf.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/dk.h \
	$(INCRT)/sys/mvme355.h \
	$(FRC)

$(LIBNAME)(mvme360.o): mvme360.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/elog.h \
	$(INCRT)/sys/iobuf.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/dk.h \
	$(INCRT)/sys/mvme360.h \
	$(FRC)

$(LIBNAME)(osm.o): osm.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/debug.h \
	$(FRC)

$(LIBNAME)(partab.o): partab.c \
	$(FRC)

$(LIBNAME)(physdsk.o): physdsk.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/elog.h \
	$(INCRT)/sys/iobuf.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/debug.h \
	$(FRC)

$(LIBNAME)(prf.o): prf.c \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/buf.h \
	$(FRC)

$(LIBNAME)(r68560.o): r68560.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/tty.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/termio.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/sysinfo.h \
	$(FRC)

$(LIBNAME)(ram.o): ram.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(FRC)

$(LIBNAME)(sem.o): sem.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/map.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/ipc.h \
	$(INCRT)/sys/sem.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/sysinfo.h \
	$(FRC)

$(LIBNAME)(shm.o): shm.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/ipc.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/shm.h \
	$(INCRT)/sys/debug.h \
	$(INCRT)/sys/tuneable.h \
	$(INCRT)/sys/cmn_err.h \
	$(FRC)

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

$(LIBNAME)(sxt.o): sxt.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/tty.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/termio.h \
	$(INCRT)/sys/ttold.h \
	$(INCRT)/sys/sxt.h \
	$(INCRT)/sys/cmn_err.h \
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

$(LIBNAME)(tpoke00.o): tpoke00.c \
	$(FRC)

$(LIBNAME)(tracker.o): tracker.c \
	$(INCRT)/sys/track.h \
	$(FRC)

$(LIBNAME)(ramdisk.o): ramdisk.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/buf.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/map.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/var.h \
	$(FRC)

$(LIBNAME)(isp.o): isp.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/tty.h \
	$(INCRT)/sys/ttold.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/termio.h \
	$(INCRT)/sys/ioctl.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/map.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/cstty.h \
	$(INCRT)/sys/csintern.h \
	$(FRC)

$(LIBNAME)(tt1.o): tt1.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/tty.h \
	$(INCRT)/sys/termio.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/cmn_err.h \
	$(INCRT)/sys/cstty.h \
	$(FRC)

$(LIBNAME)(tty.o): tty.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/immu.h \
	$(INCRT)/sys/region.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/psw.h \
	$(INCRT)/sys/systm.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/tty.h \
	$(INCRT)/sys/ttold.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/termio.h \
	$(INCRT)/sys/sysinfo.h \
	$(INCRT)/sys/map.h \
	$(INCRT)/sys/cstty.h \
	$(FRC)
