#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)libc-m32:makefile	1.43"
#
#  makefile for libc/m32
#
#
# The variable PROF is null by default, causing profiled object to be
# maintained.  If profiled object is not desired, the reassignment
# PROF=@# should appear in the make command line.
#

.SUFFIXES: .p

M4=m4 m4.def
CC=cc
CFLAGS=-O
PROF=@#
NONPROF=
INCLIBC=../inc
DEFLIST=
SDEFLIST=
INCLIST=-I$(INCLIBC)

OBJECTS=\
csu/crt0.o	  csu/mcrt0.o	    csu/initfpu.o     \
gen/abs.o	  \
gen/cuexit.o      \
gen/ecvt.o	  gen/frexp.o	    gen/gen_def.o     \
gen/ldexp.o	  \
gen/ltostr.o	  gen/memcpy.o	    gen/setjmp.o	\
gen/strcpy.o      gen/strcmp.o	    gen/sysm68k.o	\
gen/strlen.o	  sys/access.o      sys/acct.o		\
sys/alarm.o	  sys/chdir.o	    sys/chmod.o	      sys/chown.o	\
sys/chroot.o	  sys/close.o	    sys/creat.o	      sys/dup.o		\
sys/execve.o	  sys/exit.o	    sys/fcntl.o	      sys/fork.o	\
sys/fstat.o	  sys/getegid.o	    sys/geteuid.o     sys/getgid.o	\
sys/getpid.o	  sys/getppid.o	    sys/getuid.o      sys/gtty.o	\
sys/ioctl.o	  sys/kill.o	    sys/link.o	      sys/lseek.o	\
sys/locking.o	  \
sys/mknod.o	  sys/mount.o	    sys/msgsys.o      sys/nice.o	\
sys/open.o	  sys/pause.o	    sys/pipe.o	      sys/profil.o	\
sys/plock.o	  sys/uadmin.o	    \
sys/ptrace.o	  sys/read.o	    sys/sbrk.o	      sys/semsys.o	\
sys/putmsg.o      sys/getmsg.o      sys/poll.o \
sys/setgid.o	  sys/setpgrp.o	    sys/setuid.o      sys/shmsys.o	\
sys/signal.o	  sys/stat.o	    sys/stime.o	      sys/stty.o	\
sys/sync.o	  sys/_sysm68k.o    sys/syscall.o     sys/time.o	\
sys/times.o	  sys/ulimit.o	    sys/umask.o	      sys/umount.o	\
sys/uname.o	  sys/unlink.o	    sys/ustat.o	      sys/utime.o	\
sys/wait.o	  sys/write.o	    \
sys/advfs.o       sys/rdebug.o      sys/rfstart.o     \
sys/rfstop.o      sys/rfsys.o	    sys/rmount.o      sys/rumount.o     \
sys/unadvfs.o     \
sys/fstatfs.o	  sys/getdents.o    sys/mkdir.o	      \
sys/rmdir.o	  sys/statfs.o      sys/sysfs.o


POBJECTS=\
csu/crt0.p	csu/mcrt0.p	csu/initfpu.p     \
gen/abs.p	\
gen/cuexit.p	\
gen/ecvt.p	\
gen/frexp.p	gen/ldexp.p     \
gen/ltostr.p	gen/memcpy.p	 gen/setjmp.p \
gen/strcpy.p    gen/strcmp.p	\
gen/strlen.p	gen/sysm68k.p	sys/access.p    sys/acct.p	\
sys/alarm.p	sys/chdir.p	sys/chmod.p	 sys/chown.p	\
sys/chroot.p	sys/close.p	sys/creat.p	 sys/dup.p		\
sys/execve.p	sys/exit.p	sys/fcntl.p	 sys/fork.p	\
sys/fstat.p	sys/getegid.p	sys/geteuid.p    sys/getgid.p	\
sys/getpid.p	sys/getppid.p	sys/getuid.p     sys/gtty.p	\
sys/ioctl.p	sys/kill.p	sys/link.p	 sys/lseek.p	\
sys/mknod.p	sys/mount.p	sys/msgsys.p     sys/nice.p	\
sys/open.p	sys/pause.p	sys/pipe.p	 sys/profil.p	\
sys/plock.p	\
sys/ptrace.p	sys/read.p	sys/sbrk.p	 sys/semsys.p	\
sys/putmsg.p    sys/getmsg.p    sys/poll.p       \
sys/setgid.p	sys/setpgrp.p	sys/setuid.p     sys/shmsys.p	\
sys/signal.p	sys/stat.p	sys/stime.p	 sys/stty.p	\
sys/sync.p	sys/_sysm68k.p  sys/syscall.p    sys/time.p	\
sys/times.p	sys/ulimit.p	sys/umask.p	 sys/umount.p	\
sys/uname.p	sys/unlink.p	sys/ustat.p	 sys/utime.p	\
sys/advfs.p     sys/rdebug.p    \
sys/rfstart.p   sys/rfstop.p    sys/rmount.p     sys/rumount.p     \
sys/rfsys.p	sys/unadvfs.p   \
sys/fstatfs.p   sys/getdents.p  sys/mkdir.p      \
sys/rmdir.p	sys/statfs.p    sys/sysfs.p 	 \
sys/wait.p	sys/write.p	sys/uadmin.p


all:
#
# compile crt runtime routines, a makefile exists in the directory
#
	cd crt ; make -e $(IGN) nonprof
	$(PROF)cd crt ; make -e $(IGN) prof NONPROF=@#

#
# compile the rest
#
	$(PROF)make -e $(IGN) prof NONPROF=@#
	$(NONPROF)make -e $(IGN) nonprof PROF=@#

nonprof: $(OBJECTS)

prof: $(POBJECTS)

.c.o .c.p:
	$(NONPROF)@echo $*.c:
	$(NONPROF)$(CC) $(DEFLIST) $(SDEFLIST) $(INCLIST) $(CFLAGS)  -c $*.c
	$(NONPROF)mv $(*F).o $*.o
	$(PROF)@echo $*.c:
	$(PROF)$(CC) $(DEFLIST) $(INCLIST) $(CFLAGS) -c -p $*.c  && mv $(*F).o $*.p

.s.o .s.p:
	$(NONPROF)@echo $*.s:
	$(NONPROF)$(M4) $(SDEFLIST) -DMCOUNT=#  $*.s   > $(*F).s   
	$(NONPROF)$(CC) $(DEFLIST) $(CFLAGS) -c $(*F).s && mv $(*F).o $*.o
	$(PROF)@echo $*.s:
	$(PROF)$(M4) mcount.def  $*.s   > $(*F).s 
	$(PROF)$(CC) $(DEFLIST) $(CFLAGS) -c -p $(*F).s  && mv $(*F).o $*.p
	-rm $(*F).s

archive:
	#
	# Note that "archive" is invoked with libc/object as current directory.
	#
	# figure out the correct ordering for all the archive modules except
	#	cuexit and fakcu, which must go at end of archive.
	mv cuexit.o cuexit.x
#	mv kill.o kill.x
#	mv getpid.o getpid.x
	mv fakcu.o fakcu.x
	$(LORDER) *.o | tsort >objlist
	mv cuexit.x cuexit.o
#	mv kill.x kill.o
#	mv getpid.x getpid.o
	mv fakcu.x fakcu.o
	#
	# build the archive with the modules in correct order.
	$(AR) q ../lib.libc `cat objlist` fakcu.o cuexit.o
	$(PROF)#
	$(PROF)# build the profiled library archive, first renaming the
	$(PROF)#	.p (profiled object) modules to .o
	$(PROF)for i in *.p ; do mv $$i `basename $$i .p`.o ; done
	if [ "$(PROF)" != "@#" ]; \
	then \
	$(PROF)$(AR) q ../libp.libc `cat objlist` getpid.o facku.o cuexit.o ; \
	fi

clean:
	-rm -f *.o
	-rm -f *.p
	-rm -f *.s

clobber: clean
	-rm -f */*.o
	-rm -f */*.p
