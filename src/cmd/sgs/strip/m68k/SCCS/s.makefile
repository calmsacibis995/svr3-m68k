h06216
s 00014/00014/00115
d D 1.2 86/09/17 16:32:21 fnf 2 1
c Changes for release integration.
e
s 00129/00000/00000
d D 1.1 86/09/17 15:18:47 fnf 1 0
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
#ident	"@(#)strip:m32/makefile	1.14"
E 2
I 2
#ident	"@(#)strip:m68k/makefile	1.14"
E 2
#
#	BELLMAC-32 STRIP MAKEFILE
#
#
D 2
SGS=m32
E 2
I 2
SGS=m68k
E 2
OWN=bin
GRP=bin
CC=cc
CFLAGS=-O -c
D 2
ARCH=AR32WR
E 2
I 2
ARCH=AR32W
E 2
#	Set ARFORMAT to "PORTAR" for portable archive format (post 5.0)
#	Set ARFORMAT to OLDAR for pre-5.0 archive format
ARFORMAT=PORTAR
#	Set FLEX to -DFLEXNAMES for flexnames (post 6.0)
FLEX= -DFLEXNAMES
DEFLIST=-D$(ARCH) -D$(ARFORMAT) -DUNIX $(FLEX)
D 2
INCLIST=-I$(M32INC) -I$(COMINC) -I$(COMSTRIP) -I$(USRINC)
E 2
I 2
INCLIST=-I$(M68KINC) -I$(COMINC) -I$(COMSTRIP) -I$(USRINC)
E 2
ENV=
CC_CMD=$(CC) $(CFLAGS) $(DEFLIST) $(INCLIST) $(ENV)
#
LDFLAGS=-i $(FFLAG)
LD_CMD=$(CC) $(LDFLAGS)
#
LDLIBS=
#
STRIP=strip
SPFLAGS=
STRIP_CMD=$(STRIP) $(SPFLAGS)
#
LINT=lint
LINTFLAGS=-p
LINT_CMD=$(LINT) $(LINTFLAGS)
#
#	External Directories
#
ROOT=
VERSION=
USRINC= $(ROOT)/usr/$(VERSION)include
#
#	Internal Directories
#
BASE = ../..
D 2
BINDIR= $(BASE)/xenv/m32/bin
M32INC= $(BASE)/inc/m32
E 2
I 2
BINDIR= $(BASE)/xenv/m68k/bin
M68KINC= $(BASE)/inc/m68k
E 2
COMINC= $(BASE)/inc/common
COMSTRIP= $(BASE)/strip/common
#
D 2
LIBLD	= $(BASE)/libld/m32/libld.a
E 2
I 2
LIBLD	= $(BASE)/libld/m68k/libld.a
E 2
#
OBJECTS= main.o setflags.o process.o checkflags.o buildtmp.o buildfcns.o \
	copytmp.o fcns.o
#
all:	strip
#
strip:	$(OBJECTS) $(LIBLD)
	$(LD_CMD) -o strip $(OBJECTS) $(LIBLD) $(LDLIBS)
#
D 2
main.o:  $(COMSTRIP)/main.c $(COMINC)/filehdr.h $(COMINC)/ldfcn.h $(M32INC)/paths.h \
	 $(M32INC)/sgs.h
E 2
I 2
main.o:  $(COMSTRIP)/main.c $(COMINC)/filehdr.h $(COMINC)/ldfcn.h $(M68KINC)/paths.h \
	 $(M68KINC)/sgs.h
E 2
	$(CC_CMD) $(COMSTRIP)/main.c
#
D 2
setflags.o:  $(COMSTRIP)/setflags.c $(M32INC)/sgs.h $(COMSTRIP)/defs.h
E 2
I 2
setflags.o:  $(COMSTRIP)/setflags.c $(M68KINC)/sgs.h $(COMSTRIP)/defs.h
E 2
	$(CC_CMD) $(COMSTRIP)/setflags.c
#
process.o:  $(COMSTRIP)/process.c $(COMINC)/filehdr.h $(COMINC)/ldfcn.h \
D 2
	    $(COMSTRIP)/defs.h $(M32INC)/paths.h $(M32INC)/sgs.h
E 2
I 2
	    $(COMSTRIP)/defs.h $(M68KINC)/paths.h $(M68KINC)/sgs.h
E 2
	$(CC_CMD) $(COMSTRIP)/process.c
#
checkflags.o: $(COMSTRIP)/checkflags.c $(COMINC)/filehdr.h $(COMINC)/ldfcn.h
	$(CC_CMD) $(COMSTRIP)/checkflags.c
#
buildtmp.o:  $(COMSTRIP)/buildtmp.c $(COMINC)/filehdr.h $(COMINC)/ldfcn.h \
	     $(COMSTRIP)/defs.h
	$(CC_CMD) $(COMSTRIP)/buildtmp.c
#
buildfcns.o:  $(COMSTRIP)/buildfcns.c $(COMINC)/filehdr.h $(COMINC)/scnhdr.h \
	      $(COMINC)/reloc.h $(COMINC)/syms.h $(COMINC)/ldfcn.h \
D 2
	      $(COMINC)/storclass.h $(COMSTRIP)/defs.h $(M32INC)/sgs.h $(COMINC)/linenum.h
E 2
I 2
	      $(COMINC)/storclass.h $(COMSTRIP)/defs.h $(M68KINC)/sgs.h $(COMINC)/linenum.h
E 2
	$(CC_CMD) $(COMSTRIP)/buildfcns.c
#
copytmp.o:  $(COMSTRIP)/copytmp.c $(COMINC)/filehdr.h $(COMINC)/ldfcn.h \
	    $(COMSTRIP)/defs.h
	$(CC_CMD) $(COMSTRIP)/copytmp.c
#
D 2
fcns.o:  $(COMSTRIP)/fcns.c $(COMINC)/filehdr.h $(COMINC)/ldfcn.h $(M32INC)/sgs.h \
E 2
I 2
fcns.o:  $(COMSTRIP)/fcns.c $(COMINC)/filehdr.h $(COMINC)/ldfcn.h $(M68KINC)/sgs.h \
E 2
	 $(COMSTRIP)/defs.h
	$(CC_CMD) $(COMSTRIP)/fcns.c
#
#
save:	$(BINDIR)/$(SGS)strip
	-rm -f $(BINDIR)/$(SGS)strip.back
	cp $(BINDIR)/$(SGS)strip $(BINDIR)/$(SGS)strip.back
#
install:	$(BINDIR)/$(SGS)strip
#
$(BINDIR)/$(SGS)strip:	strip
# Place strip in temp location so that the correct strip command
# for installation will be used.
	mv strip strip.temp
	cp strip.temp strip.bak
	-$(STRIP_CMD) strip.temp
	sh $(BASE)/sgs.install 755 $(OWN) $(GRP) $(BINDIR)/$(SGS)strip strip.temp
# Once strip has been stripped, we can restore it from the temporary
# location.
	rm strip.temp
	mv strip.bak strip
#
uninstall:	$(BINDIR)/$(SGS)strip.back
	-rm -f $(BINDIR)/$(SGS)strip
	cp $(BINDIR)/$(SGS)strip.back $(BINDIR)/$(SGS)strip
#
lint:	$(COMSTRIP)/main.c $(COMSTRIP)/setflags.c $(COMSTRIP)/process.c \
	$(COMSTRIP)/checkflags.c $(COMSTRIP)/buildtmp.c $(COMSTRIP)/buildfcns.c \
	$(COMSTRIP)/copytmp.c $(COMSTRIP)/fcns.c \
	$(COMINC)/filehdr.h $(COMINC)/ldfcn.h $(COMINC)/scnhdr.h \
	$(COMINC)/reloc.h $(COMINC)/syms.h $(COMINC)/storclass.h \
D 2
	$(M32INC)/paths.h $(M32INC)/sgs.h $(COMSTRIP)/defs.h
E 2
I 2
	$(M68KINC)/paths.h $(M68KINC)/sgs.h $(COMSTRIP)/defs.h
E 2
	$(LINT_CMD) $(DEFLIST) $(INCLIST) $(COMSTRIP)/*.c
#
shrink:
	-rm -f $(OBJECTS) strip
E 1
