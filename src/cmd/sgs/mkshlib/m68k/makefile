#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)mkshlib:m68k/makefile	1.3"
#
#	m68k mkshlib MAKEFILE
#
#
SGS=m68k
OWN=bin
GRP=bin
CC=cc
CFLAGS=-O -c
#DBUG=
DBUG=-DNO_DBUG
DEFLIST=$(DBUG)
INCLIST=-I$(M68KINC) -I$(COMINC) -I$(COMDIR) -I$(USRINC)
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
BINDIR= $(BASE)/xenv/m68k/bin
LIBDIR= $(BASE)/xenv/m68k/lib
M68KINC= $(BASE)/inc/m68k
COMDIR= $(BASE)/mkshlib/common
M68KDIR= $(BASE)/mkshlib/m68k
COMINC= $(BASE)/inc/common
#
LIBLD	= $(BASE)/libld/m68k/libld.a
#LIBLD	= /usr/lib/libld.a
LIBDBUG=
#LIBDBUG	= /usr/local/lib/libdbug.a
OBJECTS = extn.o fcns.o main.o t_mkbt.o t_btassem.o \
	  t_mktarget.o rdspec.o \
	  h_assem.o h_create.o h_fcns.o h_mkhost.o h_pobjs.o

#
all:	mkshlib
#
mkshlib:	$(OBJECTS) $(LIBLD)
	$(LD_CMD) -o mkshlib $(OBJECTS) $(LIBLD) $(LIBDBUG) $(LDLIBS)
#
main.o:	$(COMDIR)/main.c $(COMINC)/filehdr.h $(COMINC)/ldfcn.h $(M68KINC)/paths.h \
	$(COMINC)/syms.h $(COMINC)/scnhdr.h $(COMDIR)/shlib.h $(COMDIR)/hst.h \
	$(COMDIR)/trg.h
	$(CC_CMD) $(COMDIR)/main.c
#
extn.o:	$(COMDIR)/extn.c $(COMINC)/filehdr.h $(COMINC)/ldfcn.h \
	$(COMINC)/storclass.h $(COMINC)/syms.h $(COMINC)/scnhdr.h \
	$(COMDIR)/shlib.h $(COMDIR)/hst.h $(COMDIR)/trg.h
	$(CC_CMD) $(COMDIR)/extn.c
#
fcns.o:	$(COMDIR)/fcns.c $(COMINC)/filehdr.h $(COMINC)/ldfcn.h \
	$(COMDIR)/shlib.h $(M68KINC)/paths.h
	$(CC_CMD) $(COMDIR)/fcns.c
#
h_assem.o:	$(M68KDIR)/h_assem.c $(COMINC)/filehdr.h $(COMINC)/ldfcn.h \
	$(COMINC)/syms.h $(COMINC)/scnhdr.h \
        $(COMDIR)/shlib.h $(COMDIR)/hst.h
	$(CC_CMD) $(M68KDIR)/h_assem.c
#
h_create.o:	$(COMDIR)/h_create.c $(COMINC)/filehdr.h $(COMINC)/ldfcn.h \
	$(COMINC)/storclass.h $(COMINC)/syms.h $(COMINC)/scnhdr.h \
        $(COMINC)/reloc.h $(COMDIR)/shlib.h $(COMDIR)/hst.h
	$(CC_CMD) $(COMDIR)/h_create.c
#
h_fcns.o:	$(COMDIR)/h_fcns.c $(COMINC)/filehdr.h $(COMINC)/ldfcn.h \
	$(COMINC)/syms.h $(COMINC)/scnhdr.h \
        $(COMDIR)/shlib.h $(COMDIR)/hst.h
	$(CC_CMD) $(COMDIR)/h_fcns.c
#
h_mkhost.o:	$(COMDIR)/h_mkhost.c $(COMINC)/filehdr.h $(COMINC)/ldfcn.h \
	$(COMINC)/syms.h $(COMINC)/scnhdr.h \
	$(COMDIR)/shlib.h $(COMDIR)/hst.h
	$(CC_CMD) $(COMDIR)/h_mkhost.c
#
h_pobjs.o:	$(COMDIR)/h_pobjs.c $(COMINC)/filehdr.h $(COMINC)/ldfcn.h \
	$(COMINC)/syms.h $(COMINC)/scnhdr.h $(M68KINC)/sgs.h $(COMINC)/reloc.h \
	$(COMINC)/aouthdr.h $(COMDIR)/shlib.h $(COMDIR)/hst.h
	$(CC_CMD) $(COMDIR)/h_pobjs.c
#
rdspec.o:	$(COMDIR)/rdspec.c $(COMINC)/filehdr.h $(COMINC)/ldfcn.h \
	$(M68KINC)/paths.h $(COMINC)/syms.h $(COMINC)/scnhdr.h \
	$(COMDIR)/shlib.h $(COMDIR)/hst.h $(COMDIR)/trg.h
	$(CC_CMD) $(COMDIR)/rdspec.c
#
t_btassem.o:	$(M68KDIR)/t_btassem.c $(COMINC)/filehdr.h $(COMINC)/ldfcn.h \
	$(COMDIR)/shlib.h $(COMDIR)/trg.h
	$(CC_CMD) $(M68KDIR)/t_btassem.c
#
t_mkbt.o:	$(COMDIR)/t_mkbt.c $(COMINC)/filehdr.h $(COMINC)/ldfcn.h \
	$(COMINC)/storclass.h $(COMINC)/syms.h $(COMINC)/scnhdr.h \
        $(COMDIR)/shlib.h $(COMDIR)/trg.h
	$(CC_CMD) $(COMDIR)/t_mkbt.c
#
t_mktarget.o:	$(COMDIR)/t_mktarget.c $(COMINC)/filehdr.h $(COMINC)/ldfcn.h \
	$(M68KINC)/paths.h $(COMDIR)/shlib.h $(COMDIR)/trg.h
	$(CC_CMD) $(COMDIR)/t_mktarget.c
#
save:	$(BINDIR)/$(SGS)mkshlib
	-rm -f $(BINDIR)/$(SGS)mkshlib.back
	cp $(BINDIR)/$(SGS)mkshlib $(BINDIR)/$(SGS)mkshlib.back
#
install:	$(BINDIR)/$(SGS)mkshlib
#
$(BINDIR)/$(SGS)mkshlib: mkshlib
	cp mkshlib mkshlib.bak
	-$(STRIP_CMD) mkshlib
	sh $(BASE)/sgs.install 755 $(OWN) $(GRP) $(BINDIR)/$(SGS)mkshlib mkshlib
	mv mkshlib.bak mkshlib
#
uninstall:	$(BINDIR)/$(SGS)mkshlib.back
	-rm -f $(BINDIR)/$(SGS)mkshlib
	cp $(BINDIR)/$(SGS)mkshlib.back $(BINDIR)/$(SGS)mkshlib
#
shrink:
	-rm -f $(OBJECTS) mkshlib
#
lint:	$(COMDIR)/extn.c $(COMDIR)/fcns.c $(COMDIR)/hst.h \
	$(COMDIR)/h_create.c $(COMDIR)/h_fcns.c $(COMDIR)/h_mkhost.c \
	$(COMDIR)/h_pobjs.c $(COMDIR)/main.c $(COMDIR)/rdspec.c \
	$(COMDIR)/shlib.h $(COMDIR)/trg.h $(COMDIR)/t_mkbt.c \
	$(COMDIR)/mktarget.c $(M68KDIR)/h_assem.c $(M68KDIR)/t_btassem.c \
	$(COMINC)/aouthdr.h $(COMINC)/filehdr.h $(COMINC)/ldfcn.h $(COMINC)/syms.h \
	$(COMINC)/storclass.h $(M68KINC)/paths.h $(M68KINC)/sgs.h \
	$(COMINC)/scnhdr.h
	$(LINT_CMD) $(DEFLIST) $(INCLIST) $(COMDIR)/*.c
