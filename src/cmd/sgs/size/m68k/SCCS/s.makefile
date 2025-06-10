h22825
s 00011/00011/00088
d D 1.2 86/11/18 14:31:46 fnf 2 1
c Checkpoint changes.
e
s 00099/00000/00000
d D 1.1 86/09/17 15:17:01 fnf 1 0
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
#ident	"@(#)size:m32/makefile	1.15"
E 2
I 2
#ident	"@(#)size:m68k/makefile	1.15"
E 2
#
#	BELLMAC-32 SIZE MAKEFILE
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
INCLIST=-I$(M32INC) -I$(COMINC) -I$(COMSIZE) -I$(USRINC)
E 2
I 2
INCLIST=-I$(M68KINC) -I$(COMINC) -I$(COMSIZE) -I$(USRINC)
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
COMSIZE= $(BASE)/size/common
COMINC= $(BASE)/inc/common
#
D 2
LIBLD	= $(BASE)/libld/m32/libld.a
E 2
I 2
LIBLD	= $(BASE)/libld/m68k/libld.a
E 2
OBJECTS = main.o process.o fcns.o
#
all:	size
size:	$(OBJECTS) $(LIBLD)
	$(LD_CMD) -o size $(OBJECTS) $(LIBLD) $(LDLIBS)
#
D 2
main.o:	$(COMINC)/filehdr.h $(COMINC)/ldfcn.h $(M32INC)/sgs.h $(COMSIZE)/defs.h \
E 2
I 2
main.o:	$(COMINC)/filehdr.h $(COMINC)/ldfcn.h $(M68KINC)/sgs.h $(COMSIZE)/defs.h \
E 2
	$(COMSIZE)/main.c
	$(CC_CMD) $(COMSIZE)/main.c
#
process.o:	$(COMINC)/filehdr.h $(COMINC)/scnhdr.h $(COMINC)/ldfcn.h \
		$(COMSIZE)/process.h $(COMSIZE)/defs.h $(COMSIZE)/process.c \
D 2
		$(M32INC)/sgs.h
E 2
I 2
		$(M68KINC)/sgs.h
E 2
	$(CC_CMD) $(COMSIZE)/process.c
#
fcns.o:	$(COMINC)/filehdr.h $(COMINC)/scnhdr.h $(COMINC)/ldfcn.h \
D 2
	$(M32INC)/sgs.h $(COMSIZE)/defs.h $(COMSIZE)/fcns.c
E 2
I 2
	$(M68KINC)/sgs.h $(COMSIZE)/defs.h $(COMSIZE)/fcns.c
E 2
	$(CC_CMD) $(COMSIZE)/fcns.c
#
save:	$(BINDIR)/$(SGS)size
	-rm -f $(BINDIR)/$(SGS)size.back
	cp $(BINDIR)/$(SGS)size $(BINDIR)/$(SGS)size.back
#
install:	$(BINDIR)/$(SGS)size
#
$(BINDIR)/$(SGS)size:	size
	cp size size.bak
	-$(STRIP_CMD) size
	sh $(BASE)/sgs.install 755 $(OWN) $(GRP) $(BINDIR)/$(SGS)size size
	mv size.bak size
#
uninstall:	$(BINDIR)/$(SGS)size.back
	-rm -f $(BINDIR)/$(SGS)size
	cp $(BINDIR)/$(SGS)size.back $(BINDIR)/$(SGS)size
#
shrink:
	-rm -f $(OBJECTS) size
#
lint:	$(COMSIZE)/main.c $(COMSIZE)/process.c $(COMSIZE)/fcns.c \
	$(COMINC)/filehdr.h $(COMINC)/ldfcn.h $(COMINC)/scnhdr.h \
D 2
	$(M32INC)/sgs.h $(COMSIZE)/process.h $(COMSIZE)/defs.h
E 2
I 2
	$(M68KINC)/sgs.h $(COMSIZE)/process.h $(COMSIZE)/defs.h
E 2
	$(LINT_CMD) $(DEFLIST) $(INCLIST) $(COMSIZE)/*.c
E 1
