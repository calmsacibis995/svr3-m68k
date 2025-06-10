h56840
s 00007/00004/00145
d D 1.4 86/09/17 16:29:56 fnf 4 3
c Changes for release integration.
e
s 00002/00001/00147
d D 1.3 86/08/18 08:49:39 fnf 3 2
c Checkpoint changes at first working shared library version
e
s 00021/00020/00127
d D 1.2 86/08/11 09:54:03 fnf 2 1
c Checkpoint changes to date.
e
s 00147/00000/00000
d D 1.1 86/07/31 14:38:48 fnf 1 0
c Initial copy from 5.3 distribution for AT&T 3b2.
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
#ident	"@(#)mkshlib:m32/makefile	1.3"
E 2
I 2
#ident	"@(#)mkshlib:m68k/makefile	1.3"
E 2
#
D 2
#	3B2/3B5 mkshlib MAKEFILE
E 2
I 2
#	m68k mkshlib MAKEFILE
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
D 4
DEFLIST=
E 4
I 4
#DBUG=
DBUG=-DNO_DBUG
DEFLIST=$(DBUG)
E 4
D 2
INCLIST=-I$(M32INC) -I$(COMINC) -I$(COMDIR) -I$(USRINC)
E 2
I 2
INCLIST=-I$(M68KINC) -I$(COMINC) -I$(COMDIR) -I$(USRINC)
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
COMDIR= $(BASE)/mkshlib/common
D 2
M32DIR= $(BASE)/mkshlib/m32
E 2
I 2
M68KDIR= $(BASE)/mkshlib/m68k
E 2
COMINC= $(BASE)/inc/common
#
D 2
LIBLD	= $(BASE)/libld/m32/libld.a
E 2
I 2
D 4
#(fnf) LIBLD	= $(BASE)/libld/m68k/libld.a
LIBLD	= /usr/lib/libld.a
I 3
LIBDBUG	= /usr/local/lib/libdbug.a
E 4
I 4
LIBLD	= $(BASE)/libld/m68k/libld.a
#LIBLD	= /usr/lib/libld.a
LIBDBUG=
#LIBDBUG	= /usr/local/lib/libdbug.a
E 4
E 3
E 2
OBJECTS = extn.o fcns.o main.o t_mkbt.o t_btassem.o \
	  t_mktarget.o rdspec.o \
	  h_assem.o h_create.o h_fcns.o h_mkhost.o h_pobjs.o

#
all:	mkshlib
#
mkshlib:	$(OBJECTS) $(LIBLD)
D 3
	$(LD_CMD) -o mkshlib $(OBJECTS) $(LIBLD) $(LDLIBS)
E 3
I 3
	$(LD_CMD) -o mkshlib $(OBJECTS) $(LIBLD) $(LIBDBUG) $(LDLIBS)
E 3
#
D 2
main.o:	$(COMDIR)/main.c $(COMINC)/filehdr.h $(COMINC)/ldfcn.h $(M32INC)/paths.h \
E 2
I 2
main.o:	$(COMDIR)/main.c $(COMINC)/filehdr.h $(COMINC)/ldfcn.h $(M68KINC)/paths.h \
E 2
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
D 2
	$(COMDIR)/shlib.h $(M32INC)/paths.h
E 2
I 2
	$(COMDIR)/shlib.h $(M68KINC)/paths.h
E 2
	$(CC_CMD) $(COMDIR)/fcns.c
#
D 2
h_assem.o:	$(M32DIR)/h_assem.c $(COMINC)/filehdr.h $(COMINC)/ldfcn.h \
E 2
I 2
h_assem.o:	$(M68KDIR)/h_assem.c $(COMINC)/filehdr.h $(COMINC)/ldfcn.h \
E 2
	$(COMINC)/syms.h $(COMINC)/scnhdr.h \
        $(COMDIR)/shlib.h $(COMDIR)/hst.h
D 2
	$(CC_CMD) $(M32DIR)/h_assem.c
E 2
I 2
	$(CC_CMD) $(M68KDIR)/h_assem.c
E 2
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
D 2
	$(COMINC)/syms.h $(COMINC)/scnhdr.h $(M32INC)/sgs.h $(COMINC)/reloc.h \
E 2
I 2
	$(COMINC)/syms.h $(COMINC)/scnhdr.h $(M68KINC)/sgs.h $(COMINC)/reloc.h \
E 2
	$(COMINC)/aouthdr.h $(COMDIR)/shlib.h $(COMDIR)/hst.h
	$(CC_CMD) $(COMDIR)/h_pobjs.c
#
rdspec.o:	$(COMDIR)/rdspec.c $(COMINC)/filehdr.h $(COMINC)/ldfcn.h \
D 2
	$(M32INC)/paths.h $(COMINC)/syms.h $(COMINC)/scnhdr.h \
E 2
I 2
	$(M68KINC)/paths.h $(COMINC)/syms.h $(COMINC)/scnhdr.h \
E 2
	$(COMDIR)/shlib.h $(COMDIR)/hst.h $(COMDIR)/trg.h
	$(CC_CMD) $(COMDIR)/rdspec.c
#
D 2
t_btassem.o:	$(M32DIR)/t_btassem.c $(COMINC)/filehdr.h $(COMINC)/ldfcn.h \
E 2
I 2
t_btassem.o:	$(M68KDIR)/t_btassem.c $(COMINC)/filehdr.h $(COMINC)/ldfcn.h \
E 2
	$(COMDIR)/shlib.h $(COMDIR)/trg.h
D 2
	$(CC_CMD) $(M32DIR)/t_btassem.c
E 2
I 2
	$(CC_CMD) $(M68KDIR)/t_btassem.c
E 2
#
t_mkbt.o:	$(COMDIR)/t_mkbt.c $(COMINC)/filehdr.h $(COMINC)/ldfcn.h \
	$(COMINC)/storclass.h $(COMINC)/syms.h $(COMINC)/scnhdr.h \
        $(COMDIR)/shlib.h $(COMDIR)/trg.h
	$(CC_CMD) $(COMDIR)/t_mkbt.c
#
t_mktarget.o:	$(COMDIR)/t_mktarget.c $(COMINC)/filehdr.h $(COMINC)/ldfcn.h \
D 2
	$(M32INC)/paths.h $(COMDIR)/shlib.h $(COMDIR)/trg.h
E 2
I 2
	$(M68KINC)/paths.h $(COMDIR)/shlib.h $(COMDIR)/trg.h
E 2
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
D 2
	$(COMDIR)/mktarget.c $(M32DIR)/h_assem.c $(M32DIR)/t_btassem.c \
E 2
I 2
	$(COMDIR)/mktarget.c $(M68KDIR)/h_assem.c $(M68KDIR)/t_btassem.c \
E 2
	$(COMINC)/aouthdr.h $(COMINC)/filehdr.h $(COMINC)/ldfcn.h $(COMINC)/syms.h \
D 2
	$(COMINC)/storclass.h $(M32INC)/paths.h $(M32INC)/sgs.h \
E 2
I 2
	$(COMINC)/storclass.h $(M68KINC)/paths.h $(M68KINC)/sgs.h \
E 2
	$(COMINC)/scnhdr.h
	$(LINT_CMD) $(DEFLIST) $(INCLIST) $(COMDIR)/*.c
E 1
