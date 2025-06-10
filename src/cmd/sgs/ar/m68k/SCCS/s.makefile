h09429
s 00002/00002/00070
d D 1.3 86/09/17 16:21:03 fnf 3 2
c Changes for release integration.
e
s 00006/00005/00066
d D 1.2 86/08/18 08:50:38 fnf 2 1
c Checkpoint changes at first working shared library version
e
s 00071/00000/00000
d D 1.1 86/08/14 08:29:05 fnf 1 0
c Baseline copy from 5.3 release for 3b2
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

D 2
#ident	"@(#)ar:m32/makefile	1.11"
E 2
I 2
#ident	"@(#)ar:m68k/makefile	1.11"
E 2

#***************************************************************************
#                                                                          *
#			UNIX Archive (ar) makefile			   *
#									   *
#***************************************************************************

ROOT	=
BINDIR	= $(ROOT)/bin
BASE	= ../..
COMI	= $(BASE)/inc/common
D 2
INCM32	= $(BASE)/inc/m32
E 2
I 2
INCM68K	= $(BASE)/inc/m68k
E 2
CDIR	= $(BASE)/ar/common
FLEX	= -DFLEXNAMES
ARFORMAT= PORTAR

OWN	= bin
GRP	= bin
STRIP	= strip
LP	= lp

USRINC= $(ROOT)/usr/$(VERSION)include

D 2
LIBLD	= $(BASE)/libld/m32/libld.a
E 2
I 2
D 3
#LIBLD	= $(BASE)/libld/m68k/libld.a
LIBLD	= /usr/lib/libld.a
E 3
I 3
LIBLD	= $(BASE)/libld/m68k/libld.a
#LIBLD	= /usr/lib/libld.a
E 3
E 2

CFILES = $(CDIR)/ar.c

D 2
HFILES = $(COMI)/ar.h $(COMI)/filehdr.h $(COMI)/syms.h $(INCM32)/paths.h
E 2
I 2
HFILES = $(COMI)/ar.h $(COMI)/filehdr.h $(COMI)/syms.h $(INCM68K)/paths.h
E 2

CFLAGS =  -O 
LDLIBS =
DEFLIST	= $(FLEX) -D$(ARFORMAT)
D 2
INCLIST	= -I$(COMI) -I$(INCM32) -I$(USRINC)
E 2
I 2
INCLIST	= -I$(COMI) -I$(INCM68K) -I$(USRINC)
E 2


all:	ar


ar:	$(CFILES) $(HFILES) $(LIBLD)
	$(CC) $(CFLAGS) $(DEFLIST) $(INCLIST) $(CFILES) -o ar $(LIBLD) $(LDLIBS)

install:	ar
	cp ar ar.bak
	$(STRIP) ar
	sh $(BASE)/sgs.install 755 $(OWN) $(GRP) $(BINDIR)/ar ar
	mv ar.bak ar

listing:
	pr -n makefile $(HFILES) $(CFILES) | $(LP)
	@echo "files sent to lineprinter"

clean:

flow:
	cflow $(PPFLAGS) $(CFILES) > $(OUTFIL)

flowr:
	cflow -r $(PPFLAGS) $(CFILES) > $(OUTFIL)

shrink:		clobber

clobber:	clean
	-rm -f ar
E 1
