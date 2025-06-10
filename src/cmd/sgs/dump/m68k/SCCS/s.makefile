h03578
s 00012/00012/00098
d D 1.2 86/09/17 16:25:13 fnf 2 1
c Changes for release integration.
e
s 00110/00000/00000
d D 1.1 86/09/17 14:54:13 fnf 1 0
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
#ident	"@(#)dump:m32/makefile	1.14"
E 2
I 2
#ident	"@(#)dump:m68k/makefile	1.14"
E 2
#
#	BELLMAC-32 DUMP MAKEFILE
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
ARFORMAT=PORTAR
#	Set FLEX to -DFLEXNAMES for flexnames (post 6.0)
FLEX= -DFLEXNAMES
DEFLIST=-D$(ARCH) -D$(ARFORMAT) $(FLEX)
D 2
INCLIST=-I$(COMINC) -I$(COMDUMP) -I$(M32INC) -I$(USRINC)
E 2
I 2
INCLIST=-I$(COMINC) -I$(COMDUMP) -I$(M68KINC) -I$(USRINC)
E 2
ENV=
CC_CMD=$(CC) $(CFLAGS) $(DEFLIST) $(INCLIST) $(ENV)
FFLAG=
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
ROOT=
VERSION=
USRINC= $(ROOT)/usr/$(VERSION)include
#
#
#	Internal Directories
BASE= ../..
D 2
BINDIR= $(BASE)/xenv/m32/bin
E 2
I 2
BINDIR= $(BASE)/xenv/m68k/bin
E 2
COMDUMP= $(BASE)/dump/common
COMINC= $(BASE)/inc/common
D 2
M32INC= $(BASE)/inc/m32
E 2
I 2
M68KINC= $(BASE)/inc/m68k
E 2
#
D 2
LIBLD	= $(BASE)/libld/m32/libld.a
E 2
I 2
LIBLD	= $(BASE)/libld/m68k/libld.a
E 2
#
OBJECTS= main.o setflags.o openobject.o process.o fcns.o
#
all:	dump
#
dump:	$(OBJECTS) $(LIBLD)
	$(LD_CMD) -o dump $(OBJECTS) $(LIBLD) $(LDLIBS)
#
main.o:	$(COMDUMP)/main.c $(COMINC)/filehdr.h $(COMINC)/ldfcn.h $(COMDUMP)/filedefs.h \
D 2
	 $(COMDUMP)/dumpmap.h $(M32INC)/sgs.h
E 2
I 2
	 $(COMDUMP)/dumpmap.h $(M68KINC)/sgs.h
E 2
	$(CC_CMD) $(COMDUMP)/main.c
#
D 2
setflags.o:	$(COMDUMP)/setflags.c $(COMDUMP)/flagdefs.h $(M32INC)/sgs.h
E 2
I 2
setflags.o:	$(COMDUMP)/setflags.c $(COMDUMP)/flagdefs.h $(M68KINC)/sgs.h
E 2
	$(CC_CMD) $(COMDUMP)/setflags.c
#
openobject.o:	$(COMDUMP)/openobject.c $(COMINC)/filehdr.h $(COMINC)/ldfcn.h \
D 2
	       $(M32INC)/sgs.h $(COMDUMP)/filedefs.h
E 2
I 2
	       $(M68KINC)/sgs.h $(COMDUMP)/filedefs.h
E 2
	$(CC_CMD) $(COMDUMP)/openobject.c
#
D 2
process.o:	$(COMDUMP)/process.c $(COMINC)/filehdr.h $(COMINC)/ldfcn.h $(M32INC)/sgs.h\
E 2
I 2
process.o:	$(COMDUMP)/process.c $(COMINC)/filehdr.h $(COMINC)/ldfcn.h $(M68KINC)/sgs.h\
E 2
	    $(COMDUMP)/filedefs.h $(COMDUMP)/process.h $(COMDUMP)/flagdefs.h
	$(CC_CMD) $(COMDUMP)/process.c
#
fcns.o:	$(COMDUMP)/fcns.c $(COMINC)/filehdr.h $(COMINC)/scnhdr.h $(COMINC)/reloc.h \
	 $(COMINC)/linenum.h $(COMINC)/syms.h $(COMINC)/storclass.h $(COMINC)/ldfcn.h \
D 2
	 $(M32INC)/sgs.h
E 2
I 2
	 $(M68KINC)/sgs.h
E 2
	$(CC_CMD) $(COMDUMP)/fcns.c
#
save:	$(BINDIR)/$(SGS)dump
	-rm -f $(BINDIR)/$(SGS)dump.back
	cp $(BINDIR)/$(SGS)dump $(BINDIR)/$(SGS)dump.back
#
install:	$(BINDIR)/$(SGS)dump
#
$(BINDIR)/$(SGS)dump:	dump
	cp dump dump.bak
	-$(STRIP_CMD) dump
	sh $(BASE)/sgs.install 755 $(OWN) $(GRP) $(BINDIR)/$(SGS)dump dump
	mv dump.bak dump
#
uninstall:	$(BINDIR)/$(SGS)dump.back
	-rm -f $(BINDIR)/$(SGS)dump
	cp $(BINDIR)/$(SGS)dump.back $(BINDIR)/$(SGS)dump
#
lint:	$(COMDUMP)/main.c $(COMDUMP)/setflags.c $(COMDUMP)/openobject.c \
	$(COMDUMP)/process.c $(COMDUMP)/fcns.c \
	$(COMDUMP)/dumpmap.h \
	$(COMINC)/filehdr.h $(COMINC)/ldfcn.h $(COMDUMP)/filedefs.h \
D 2
	$(M32INC)/sgs.h $(COMDUMP)/flagdefs.h $(M32INC)/paths.h \
E 2
I 2
	$(M68KINC)/sgs.h $(COMDUMP)/flagdefs.h $(M68KINC)/paths.h \
E 2
	$(COMINC)/syms.h $(COMINC)/storclass.h $(COMINC)/linenum.h \
	$(COMINC)/scnhdr.h $(COMINC)/reloc.h
	$(LINT_CMD) $(DEFLIST) $(INCLIST) $(COMDUMP)/*.c
#
shrink:
	-rm -f $(OBJECTS) dump
E 1
