h58978
s 00009/00009/00034
d D 1.2 86/08/18 08:52:25 fnf 2 1
c Checkpoint changes at first working shared library version
e
s 00043/00000/00000
d D 1.1 86/07/31 10:17:54 fnf 1 0
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

#ident	"@(#)libc-port:makefile	1.17"
#
# makefile for libc/port
#
#
# The macro PROF is null by default, causing profiled object to be maintained.
# If profiled object is not desired, the reassignment PROF=@# should appear in
# the make command line.
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
DEFLIST=
SDEFLIST=

all:	prof nonprof 

nonprof:
D 2
	$(NONPROF)cd gen; make -e $(IGN) nonprof PROF=@#
	$(NONPROF)cd print; make -e $(IGN) nonprof PROF=@#
	$(NONPROF)cd stdio; make -e $(IGN) nonprof PROF=@#
	$(NONPROF)cd sys; make -e $(IGN) nonprof PROF=@#
E 2
I 2
	$(NONPROF)cd gen; $(MAKE) -e $(IGN) nonprof PROF=@#
	$(NONPROF)cd print; $(MAKE) -e $(IGN) nonprof PROF=@#
	$(NONPROF)cd stdio; $(MAKE) -e $(IGN) nonprof PROF=@#
	$(NONPROF)cd sys; $(MAKE) -e $(IGN) nonprof PROF=@#
E 2
prof:
D 2
	$(PROF)cd gen; make -e $(IGN) prof NONPROF=@#
	$(PROF)cd print; make -e $(IGN) prof NONPROF=@#
	$(PROF)cd stdio; make -e $(IGN) prof NONPROF=@#
	$(PROF)cd sys; make -e $(IGN) prof NONPROF=@#
E 2
I 2
	$(PROF)cd gen; $(MAKE) -e $(IGN) prof NONPROF=@#
	$(PROF)cd print; $(MAKE) -e $(IGN) prof NONPROF=@#
	$(PROF)cd stdio; $(MAKE) -e $(IGN) prof NONPROF=@#
	$(PROF)cd sys; $(MAKE) -e $(IGN) prof NONPROF=@#
E 2
	
clean:

clobber: clean
	-rm -f */*.o
	-rm -f */*.p
E 1
