h06846
s 00013/00012/00101
d D 1.2 86/09/17 16:31:43 fnf 2 1
c Changes for release integration.
e
s 00113/00000/00000
d D 1.1 86/08/19 09:11:16 fnf 1 0
c date and time created 86/08/19 09:11:16 by fnf
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
#ident	"@(#)nm:m32/makefile	1.16"
E 2
I 2
#ident	"@(#)nm:m68k/makefile	1.16"
E 2
#
D 2
#	BELLMAC-32 NM MAKEFILE
E 2
I 2
#	MOTOROLA-M68K NM MAKEFILE
E 2
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
ARCH=AR32WR
#	Set ARFORMAT to "PORTAR" for portable archive format (post 5.0)
#	Set ARFORMAT to OLDAR for pre-5.0 archive format
ARFORMAT=PORTAR
#	Set FLEX to -DFLEXNAMES for flexnames (post 6.0)
FLEX= -DFLEXNAMES
DEFLIST=-D$(ARCH) -D$(ARFORMAT) -DUNIX $(FLEX)
D 2
INCLIST=-I$(M32INC) -I$(COMINC) -I$(COMNM) -I$(USRINC)
E 2
I 2
INCLIST=-I$(M68KINC) -I$(COMINC) -I$(COMNM) -I$(USRINC)
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
BASE= ../..
D 2
BINDIR= $(BASE)/xenv/m32/bin
LIBDIR= $(BASE)/xenv/m32/lib
M32INC= $(BASE)/inc/m32
E 2
I 2
BINDIR= $(BASE)/xenv/m68k/bin
LIBDIR= $(BASE)/xenv/m68k/lib
M68KINC= $(BASE)/inc/m68k
E 2
COMNM= $(BASE)/nm/common
COMINC= $(BASE)/inc/common
#
D 2
LIBLD	= $(BASE)/libld/m32/libld.a
E 2
I 2
LIBLD	= $(BASE)/libld/m68k/libld.a
#LIBLD	= /usr/lib/libld.a
E 2
OBJECTS = main.o setflags.o process.o prosym.o decotype.o fcns.o
#
all:	nm
#
nm:	$(OBJECTS) $(LIBLD)
	$(LD_CMD) -o nm $(OBJECTS) $(LIBLD) $(LDLIBS)
#
D 2
main.o:	$(COMNM)/main.c $(COMINC)/filehdr.h $(COMINC)/ldfcn.h $(M32INC)/sgs.h \
E 2
I 2
main.o:	$(COMNM)/main.c $(COMINC)/filehdr.h $(COMINC)/ldfcn.h $(M68KINC)/sgs.h \
E 2
	$(COMNM)/defs.h
	$(CC_CMD) $(COMNM)/main.c
#
D 2
setflags.o:	$(COMNM)/setflags.c $(M32INC)/sgs.h $(M32INC)/paths.h $(COMNM)/defs.h
E 2
I 2
setflags.o:	$(COMNM)/setflags.c $(M68KINC)/sgs.h $(M68KINC)/paths.h $(COMNM)/defs.h
E 2
	$(CC_CMD) $(COMNM)/setflags.c
#
process.o:	$(COMNM)/process.c $(COMINC)/filehdr.h $(COMINC)/ldfcn.h \
		$(COMINC)/scnhdr.h $(COMNM)/defs.h
	$(CC_CMD) $(COMNM)/process.c
#
prosym.o:	$(COMNM)/prosym.c $(COMINC)/filehdr.h $(COMINC)/ldfcn.h $(COMINC)/syms.h \
		$(COMINC)/storclass.h $(COMNM)/defs.h $(COMNM)/prosym.h
	$(CC_CMD) $(COMNM)/prosym.c
#
decotype.o:	$(COMNM)/decotype.c $(COMINC)/filehdr.h $(COMINC)/ldfcn.h \
		$(COMINC)/syms.h $(COMINC)/storclass.h $(COMNM)/decotype.h $(COMNM)/defs.h
	$(CC_CMD) $(COMNM)/decotype.c
#
D 2
fcns.o:	$(COMNM)/fcns.c $(COMINC)/filehdr.h $(COMINC)/ldfcn.h $(M32INC)/sgs.h \
E 2
I 2
fcns.o:	$(COMNM)/fcns.c $(COMINC)/filehdr.h $(COMINC)/ldfcn.h $(M68KINC)/sgs.h \
E 2
	$(COMNM)/defs.h
	$(CC_CMD) $(COMNM)/fcns.c
#
save:	$(BINDIR)/$(SGS)nm
	-rm -f $(BINDIR)/$(SGS)nm.back
	cp $(BINDIR)/$(SGS)nm $(BINDIR)/$(SGS)nm.back
#
install:	$(BINDIR)/$(SGS)nm
#
$(BINDIR)/$(SGS)nm: nm
	cp nm nm.bak
	-$(STRIP_CMD) nm
	sh $(BASE)/sgs.install 755 $(OWN) $(GRP) $(BINDIR)/$(SGS)nm nm
	mv nm.bak nm
#
uninstall:	$(BINDIR)/$(SGS)nm.back
	-rm -f $(BINDIR)/$(SGS)nm
	cp $(BINDIR)/$(SGS)nm.back $(BINDIR)/$(SGS)nm
#
shrink:
	-rm -f $(OBJECTS) nm
#
lint:	$(COMNM)/main.c $(COMNM)/setflags.c $(COMNM)/process.c \
	$(COMNM)/prosym.c $(COMNM)/decotype.c $(COMNM)/fcns.c \
	$(COMINC)/filehdr.h $(COMINC)/ldfcn.h $(COMINC)/syms.h \
D 2
	$(COMINC)/storclass.h $(M32INC)/paths.h $(M32INC)/sgs.h \
E 2
I 2
	$(COMINC)/storclass.h $(M68KINC)/paths.h $(M68KINC)/sgs.h \
E 2
	$(COMNM)/prosym.h $(COMNM)/decotype.h \
	$(COMINC)/scnhdr.h $(COMS/defs.h
	$(LINT_CMD) $(DEFLIST) $(INCLIST) $(COMNM)/*.c
E 1
