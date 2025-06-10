h23790
s 00108/00095/00056
d D 1.2 86/07/31 12:37:21 fnf 2 1
c Pick up Matt's version for 5.3 m68k libc.
e
s 00151/00000/00000
d D 1.1 86/07/31 12:34:54 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
D 2
#	@(#)makefile	7.2 
E 2
I 2
#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)libc-m32:makefile	1.43"
E 2
#
D 2
# makefile for libc/m68k
E 2
I 2
#  makefile for libc/m32
E 2
#
#
# The variable PROF is null by default, causing profiled object to be
# maintained.  If profiled object is not desired, the reassignment
# PROF=@# should appear in the make command line.
#

D 2
ROOT =

E 2
.SUFFIXES: .p

M4=m4 m4.def
CC=cc
CFLAGS=-O
D 2
PROF=
PROT=664
GRP=bin
OWN=bin
E 2
I 2
PROF=@#
NONPROF=
INCLIBC=../inc
DEFLIST=
SDEFLIST=
INCLIST=-I$(INCLIBC)
E 2

D 2
# a makefile exists in crt and csu for m68k
#

E 2
OBJECTS=\
D 2
gen/abs.o         gen/cuexit.o      gen/setjmp.o	    \
gen/ecvt.o	  gen/frexp.o	    gen/modf.o				\
gen/memccpy.o     gen/memchr.o      gen/memcmp.o      gen/memcpy.o      \
gen/memset.o \
gen/strcat.o      gen/strchr.o      gen/strcmp.o      gen/strcpy.o      \
gen/strcspn.o     gen/strlen.o      gen/strncat.o     gen/strncmp.o     \
gen/strncpy.o     gen/strpbrk.o     gen/strrchr.o     gen/strspn.o      \
sys/access.o      sys/acct.o        sys/alarm.o       sys/chdir.o	\
sys/chmod.o       sys/chown.o       sys/chroot.o      sys/close.o	\
sys/creat.o       sys/dup.o         \
sys/execve.o      sys/exit.o        sys/fcntl.o	\
sys/fork.o        sys/fstat.o       sys/getegid.o     sys/geteuid.o	\
sys/getgid.o      sys/getpid.o      sys/getppid.o     sys/getuid.o	\
sys/gtty.o        sys/ioctl.o       sys/kill.o        sys/link.o	\
sys/locking.o	  sys/lseek.o       sys/mknod.o       sys/mount.o       \
sys/nice.o        sys/open.o        sys/pause.o       sys/pipe.o	\
sys/plock.o       sys/profil.o      sys/ptrace.o      sys/read.o	\
sys/sbrk.o        sys/setgid.o      sys/setpgrp.o     sys/setuid.o	\
sys/signal.o      sys/stat.o        sys/stime.o	      \
sys/stty.o        sys/sync.o        sys/syscall.o     sys/time.o	\
sys/_sysm68k.o    \
sys/times.o       sys/ulimit.o      sys/umask.o       sys/umount.o	\
sys/uname.o       sys/unlink.o      sys/ustat.o       sys/utime.o	\
sys/wait.o        sys/write.o
E 2
I 2
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
E 2

I 2

E 2
POBJECTS=\
D 2
gen/abs.p         gen/cuexit.p      gen/setjmp.p	\
gen/ecvt.p	  gen/frexp.p	    gen/modf.p				\
gen/memccpy.p     gen/memchr.p      gen/memcmp.p      gen/memcpy.p      \
gen/memset.p \
gen/strcat.p      gen/strchr.p      gen/strcmp.p      gen/strcpy.p      \
gen/strcspn.p     gen/strlen.p      gen/strncat.p     gen/strncmp.p     \
gen/strncpy.p     gen/strpbrk.p     gen/strrchr.p     gen/strspn.p      \
sys/access.p      sys/acct.p        sys/alarm.p       sys/chdir.p	\
sys/chmod.p       sys/chown.p       sys/chroot.p      sys/close.p	\
sys/creat.p       sys/dup.p         \
sys/execve.p      sys/exit.p        sys/fcntl.p	\
sys/fork.p        sys/fstat.p       sys/getegid.p     sys/geteuid.p	\
sys/getgid.p      sys/getpid.p      sys/getppid.p     sys/getuid.p	\
sys/gtty.p        sys/ioctl.p       sys/kill.p        sys/link.p	\
sys/lseek.p       sys/mknod.p       sys/mount.p       \
sys/nice.p        sys/open.p        sys/pause.p       sys/pipe.p	\
sys/plock.p       sys/profil.p      sys/ptrace.p      sys/read.p	\
sys/sbrk.p        sys/setgid.p      sys/setpgrp.p     sys/setuid.p	\
sys/signal.p      sys/stat.p        sys/stime.p	      \
sys/stty.p        sys/sync.p        sys/syscall.p     sys/time.p	\
sys/_sysm68k.p    \
sys/times.p       sys/ulimit.p      sys/umask.p       sys/umount.p	\
sys/uname.p       sys/unlink.p      sys/ustat.p       sys/utime.p	\
sys/wait.p        sys/write.p
E 2
I 2
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
E 2


all:
#
# compile crt runtime routines, a makefile exists in the directory
#
	cd crt ; make -e $(IGN) nonprof
	$(PROF)cd crt ; make -e $(IGN) prof NONPROF=@#
I 2

E 2
#
D 2
# compile csu start up routines, a makefile exists in the directory
E 2
I 2
# compile the rest
E 2
#
D 2
	cd csu ; make -e $(IGN) nonprof
	$(PROF)cd csu ; make -e $(IGN) prof 
#
# compile gen and sys directories
#
	make -e $(IGN) nonprof
E 2
	$(PROF)make -e $(IGN) prof NONPROF=@#
D 2
	
nonprof: $(OBJECTS) gen/sysm68k.o
E 2
I 2
	$(NONPROF)make -e $(IGN) nonprof PROF=@#
E 2

D 2
prof: $(POBJECTS) gen/sysm68k.p
E 2
I 2
nonprof: $(OBJECTS)
E 2

I 2
prof: $(POBJECTS)

E 2
.c.o .c.p:
D 2
	@echo $*.c:
	$(NONPROF)$(CC) $(CFLAGS) -c $*.c  && mv $(*F).o $*.o
	$(PROF)$(CC) -p $(CFLAGS) -c $*.c  && mv $(*F).o $*.p
E 2
I 2
	$(NONPROF)@echo $*.c:
	$(NONPROF)$(CC) $(DEFLIST) $(SDEFLIST) $(INCLIST) $(CFLAGS)  -c $*.c
	$(NONPROF)mv $(*F).o $*.o
	$(PROF)@echo $*.c:
	$(PROF)$(CC) $(DEFLIST) $(INCLIST) $(CFLAGS) -c -p $*.c  && mv $(*F).o $*.p
E 2

.s.o .s.p:
D 2
	@echo $*.s:
	$(NONPROF)$(M4) -DMCOUNT=#  $*.s   > $(*F).s   && \
	$(NONPROF)$(CC) $(CFLAGS) -c $(*F).s  && mv $(*F).o $*.o
	$(PROF)$(M4) mcount.def  $*.s   > $(*F).s   && $(CC) $(CFLAGS) -c $(*F).s  && mv $(*F).o $*.p
E 2
I 2
	$(NONPROF)@echo $*.s:
	$(NONPROF)$(M4) $(SDEFLIST) -DMCOUNT=#  $*.s   > $(*F).s   
	$(NONPROF)$(CC) $(DEFLIST) $(CFLAGS) -c $(*F).s && mv $(*F).o $*.o
	$(PROF)@echo $*.s:
	$(PROF)$(M4) mcount.def  $*.s   > $(*F).s 
	$(PROF)$(CC) $(DEFLIST) $(CFLAGS) -c -p $(*F).s  && mv $(*F).o $*.p
E 2
	-rm $(*F).s

D 2
gen/sysm68k.o gen/sysm68k.p:
	@echo $*.c:
	$(NONPROF)M68000= STACKCHECK=OFF $(CC) $(CFLAGS) -c $*.c  && mv $(*F).o $*.o
	$(PROF)M68000= STACKCHECK=OFF $(CC) -p $(CFLAGS) -c $*.c  && mv $(*F).o $*.p
	
E 2
archive:
	#
	# Note that "archive" is invoked with libc/object as current directory.
	#
	# figure out the correct ordering for all the archive modules except
	#	cuexit and fakcu, which must go at end of archive.
D 2
	chmod $(PROT) *.o 
	chgrp $(GRP)  *.o
	chown $(OWN)  *.o
E 2
	mv cuexit.o cuexit.x
I 2
#	mv kill.o kill.x
#	mv getpid.o getpid.x
E 2
	mv fakcu.o fakcu.x
	$(LORDER) *.o | tsort >objlist
	mv cuexit.x cuexit.o
I 2
#	mv kill.x kill.o
#	mv getpid.x getpid.o
E 2
	mv fakcu.x fakcu.o
	#
	# build the archive with the modules in correct order.
D 2
	$(AR) q ../lib.libc `cat objlist` cuexit.o fakcu.o
	$(AR) t ../lib.libc >../lib.contents
	rm -f *.o
E 2
I 2
	$(AR) q ../lib.libc `cat objlist` fakcu.o cuexit.o
E 2
	$(PROF)#
	$(PROF)# build the profiled library archive, first renaming the
	$(PROF)#	.p (profiled object) modules to .o
	$(PROF)for i in *.p ; do mv $$i `basename $$i .p`.o ; done
D 2
	$(PROF)chmod $(PROT) *.o
	$(PROF)chgrp $(GRP)  *.o
	$(PROF)chown $(OWN)  *.o
	$(PROF)$(AR) q ../libp.libc `cat objlist` cuexit.o fakcu.o
	$(PROF)$(STRIP) -r ../libp.libc
	$(PROF)$(AR) ts ../libp.libc >../libp.contents
E 2
I 2
	if [ "$(PROF)" != "@#" ]; \
	then \
	$(PROF)$(AR) q ../libp.libc `cat objlist` getpid.o facku.o cuexit.o ; \
	fi
E 2

clean:
D 2
	-rm -f *.[ops]
E 2
I 2
	-rm -f *.o
	-rm -f *.p
	-rm -f *.s
E 2

clobber: clean
D 2
	-rm -f */*.[op]
E 2
I 2
	-rm -f */*.o
	-rm -f */*.p
E 2
E 1
