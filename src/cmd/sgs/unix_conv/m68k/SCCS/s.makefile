h56272
s 00006/00006/00093
d D 1.2 86/09/17 16:33:14 fnf 2 1
c Changes for release integration.
e
s 00099/00000/00000
d D 1.1 86/09/17 15:20:55 fnf 1 0
c From Unix 5.3 release for 3b2
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
#ident	"@(#)unix_conv:m32/makefile	1.10"
E 2
I 2
#ident	"@(#)unix_conv:m68k/makefile	1.10"
E 2

#***************************************************************************
#                                                                          *
#		UNIX 5.0 Transition Aid (convert) makefile	 	   *
#									   *
#***************************************************************************
#                                                                          *
#		Header/Data Files					   *
#                                                                          *
#	inc/ar.h		archive file format header		   *
#	inc/a.out.h		machine language file format header        *
#	inc/sgs.h		sgs primitives				   *
#	com/old.a.out.h		pre 5.0 headers for a.out.h		   *
#	com/old.ar.h		pre 5.0 headers for ar.h		   *
#                                                                          *
#***************************************************************************
#                                                                          *
#		Source Code Files					   *
#								           *
#	com/convert.c		transition aid source code		   *
#									   *
#***************************************************************************

LPT = dpr -mx

OUTFIL = output.tmp

OWN	= bin
GRP	= bin
ROOT	=
BINDIR	= $(ROOT)/bin

COMI = ../../inc/common
CDIR = ../common
D 2
INCM32 = ../../inc/m32
E 2
I 2
INCM68K = ../../inc/m68k
E 2

LDLIBS=

OFILES = convert.o

CFILES = $(CDIR)/convert.c

HFILES = $(COMI)/ar.h $(COMI)/aouthdr.h $(COMI)/filehdr.h \
	 $(COMI)/scnhdr.h $(COMI)/linenum.h $(COMI)/reloc.h \
	 $(COMI)/syms.h $(COMI)/storclass.h \
	 $(CDIR)/old.a.out.h $(CDIR)/old.ar.h $(CDIR)/5.0.ar.h \
D 2
	 $(INCM32)/paths.h
E 2
I 2
	 $(INCM68K)/paths.h
E 2

BASE	= ../..
D 2
LIBLD	= $(BASE)/libld/m32/libld.a
E 2
I 2
LIBLD	= $(BASE)/libld/m68k/libld.a
E 2
STRIP	= strip

FLEX	= -DFLEXNAMES
D 2
DEFLIST	= -DM32 $(FLEX) -D$(ARFORMAT)
INCLIST	= -I$(CDIR) -I$(COMI) -I$(INCM32)
E 2
I 2
DEFLIST	= -DM68k $(FLEX) -D$(ARFORMAT)
INCLIST	= -I$(CDIR) -I$(COMI) -I$(INCM68K)
E 2

CFLAGS = -O

ARFORMAT=PORTAR


all:	convert


convert:$(CFILES) $(HFILES) $(LIBLD)
	$(CC) $(DEFLIST) $(INCLIST) $(CFLAGS) $(CFILES) $(LIBLD) $(LDLIBS) -o convert


install:	convert
	cp convert convert.bak
	$(STRIP) convert
	sh $(BASE)/sgs.install 755 $(OWN) $(GRP) $(BINDIR)/convert convert
	mv convert.bak convert

listing:
	pr -n makefile $(HFILES) $(CFILES) | $(LPT)
	@echo "files sent to lineprinter"

clean:
	-rm -f $(OFILES)

flow:
	cflow $(PPFLAGS) $(CFILES) > $(OUTFIL)

flowr:
	cflow -r $(PPFLAGS) $(CFILES) > $(OUTFIL)

shrink:  clobber

clobber: clean
	-rm -f convert
E 1
