h60821
s 00005/00004/00050
d D 1.2 86/08/18 08:52:20 fnf 2 1
c Checkpoint changes at first working shared library version
e
s 00054/00000/00000
d D 1.1 86/07/31 11:47:14 fnf 1 0
c Initial copy from 5.3 distribution for 3b2.
e
u
U
t
T
I 1
#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)libc-port:sys/makefile	1.5.1.7"
#
# makefile for libc/port/sys
#
#

.SUFFIXES: .p
CC=cc
CFLAGS=-O
D 2
PROF=
E 2
I 2
PROF=@#
E 2
NONPROF=
D 2
ROOT=
E 2
I 2
ROOT=/aq1
E 2
INC=$(ROOT)/usr/include
INCSYS=$(ROOT)/usr/include/sys
INCLIBC=../../inc
DEFLIST=
SDEFLIST=
INCLIST=-I$(INCLIBC)

OBJECTS=\
execl.o       execle.o      execv.o	lockf.o    \
D 2
msgsys.o      semsys.o      shmsys.o  
E 2
I 2
msgsys.o      semsys.o      shmsys.o    sigcall.o
E 2

POBJECTS=\
execl.p       execle.p      execv.p	lockf.p    \
D 2
msgsys.p      semsys.p      shmsys.p  
E 2
I 2
msgsys.p      semsys.p      shmsys.p    sigcall.p
E 2

nonprof: $(OBJECTS)

prof: $(POBJECTS)

msgsys.o msgsys.p semsys.o semsys.p \
	shmsys.o shmsys.p: $(INCSYS)/ipc.h
msgsys.o msgsys.p: $(INCSYS)/msg.h
semsys.o semsys.p: $(INCSYS)/sem.h
shmsys.o shmsys.p: $(INCSYS)/shm.h
msgsys.o msgsys.p semsys.o semsys.p \
	shmsys.o shmsys.p: $(INCSYS)/types.h
lockf.o lockf.p: $(INC)/errno.h 
lockf.o lockf.p: $(INC)/fcntl.h 
lockf.o lockf.p: $(INC)/unistd.h
I 2
sigcall.o sigcall.p: $(INC)/signal.h $(INCSYS)/signal.h
E 2

.c.o .c.p:
	$(NONPROF)@echo $*.c:
	$(NONPROF)$(CC) $(DEFLIST) $(SDEFLIST) $(INCLIST) $(CFLAGS) -c $*.c
	$(PROF)@echo $*.c:
	$(PROF)$(CC) $(DEFLIST) $(INCLIST) $(CFLAGS) -c -p $*.c  && mv $(*F).o $*.p
E 1
