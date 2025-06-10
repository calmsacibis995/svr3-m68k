h18577
s 00012/00012/00108
d D 1.2 86/09/17 16:27:40 fnf 2 1
c Changes for release integration.
e
s 00120/00000/00000
d D 1.1 86/09/17 15:05:33 fnf 1 0
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
#ident	"@(#)list:m32/makefile	1.14"
E 2
I 2
#ident	"@(#)list:m68k/makefile	1.14"
E 2
#
#	BELLMAC-32 LIST MAKEFILE
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
ARCH=AR32W
#	Set FLEX to -DFLEXNAMES for flexnames
FLEX= -DFLEXNAMES
DEFLIST=-D$(ARCH) $(FLEX)
D 2
INCLIST=-I$(M32INC) -I$(COMINC) -I$(COMLIST) -I$(USRINC)
E 2
I 2
INCLIST=-I$(M68KINC) -I$(COMINC) -I$(COMLIST) -I$(USRINC)
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
COMLIST= $(BASE)/list/common
COMINC= $(BASE)/inc/common
#
#	External Directories
#
ROOT=
VERSION=
USRINC=$(ROOT)/usr/$(VERSION)include
#
D 2
LIBLD	= $(BASE)/libld/m32/libld.a
E 2
I 2
LIBLD	= $(BASE)/libld/m68k/libld.a
E 2
OBJECTS= main.o initfcns.o process.o symfcns.o findname.o parsefcns.o \
	srciofcns.o fcns.o
#
all:	list
#
list:		$(OBJECTS) $(LIBLD)
	$(LD_CMD) -o list $(OBJECTS) $(LIBLD) $(LDLIBS)
#
D 2
main.o:	$(COMLIST)/main.c $(COMINC)/filehdr.h $(COMINC)/ldfcn.h $(M32INC)/sgs.h \
E 2
I 2
main.o:	$(COMLIST)/main.c $(COMINC)/filehdr.h $(COMINC)/ldfcn.h $(M68KINC)/sgs.h \
E 2
	$(COMLIST)/defs.h
	$(CC_CMD) $(COMLIST)/main.c
#
initfcns.o:	$(COMLIST)/initfcns.c $(COMINC)/filehdr.h $(COMINC)/ldfcn.h \
D 2
		$(M32INC)/paths.h $(M32INC)/sgs.h $(COMLIST)/defs.h
E 2
I 2
		$(M68KINC)/paths.h $(M68KINC)/sgs.h $(COMLIST)/defs.h
E 2
	$(CC_CMD) $(COMLIST)/initfcns.c
#
process.o:	$(COMLIST)/process.c $(COMLIST)/defs.h
	$(CC_CMD) $(COMLIST)/process.c
#
symfcns.o:	$(COMLIST)/symfcns.c $(COMINC)/filehdr.h $(COMINC)/syms.h $(COMINC)/ldfcn.h \
		$(COMINC)/storclass.h $(COMLIST)/defs.h
	$(CC_CMD) $(COMLIST)/symfcns.c
#
findname.o:	$(COMLIST)/findname.c $(COMINC)/syms.h $(COMINC)/storclass.h \
		$(COMLIST)/defs.h $(COMLIST)/findname.h
	$(CC_CMD) $(COMLIST)/findname.c
#
parsefcns.o:	$(COMLIST)/parsefcns.c $(COMLIST)/defs.h
	$(CC_CMD) $(COMLIST)/parsefcns.c
#
srciofcns.o:	$(COMLIST)/srciofcns.c $(COMINC)/filehdr.h $(COMINC)/linenum.h \
		$(COMINC)/ldfcn.h $(COMLIST)/defs.h $(COMINC)/sgsmacros.h
	$(CC_CMD) $(COMLIST)/srciofcns.c
#
D 2
fcns.o:	$(COMLIST)/fcns.c $(COMINC)/filehdr.h $(COMINC)/ldfcn.h $(M32INC)/sgs.h \
E 2
I 2
fcns.o:	$(COMLIST)/fcns.c $(COMINC)/filehdr.h $(COMINC)/ldfcn.h $(M68KINC)/sgs.h \
E 2
	$(COMLIST)/defs.h
	$(CC_CMD) $(COMLIST)/fcns.c
#
#
shrink:
	-rm -f $(OBJECTS) list
#
save:	$(BINDIR)/$(SGS)list
	-rm -f $(BINDIR)/$(SGS)list.back
	cp $(BINDIR)/$(SGS)list $(BINDIR)/$(SGS)list.back
#
install:	$(BINDIR)/$(SGS)list
#
$(BINDIR)/$(SGS)list:	list
	cp list list.bak
	-$(STRIP_CMD) list
	sh $(BASE)/sgs.install 755 $(OWN) $(GRP) $(BINDIR)/$(SGS)list list
	mv list.bak list
#
uninstall:	$(BINDIR)/$(SGS)list.back
	-rm -f $(BINDIR)/$(SGS)list
	cp $(BINDIR)/$(SGS)list.back $(BINDIR)/$(SGS)list
#
lint:	$(COMLIST)/main.c $(COMLIST)/initfcns.c $(COMLIST)/process.c \
	$(COMLIST)/symfcns.c $(COMLIST)/findname.c $(COMLIST)/parsefcns.c \
	$(COMLIST)/srciofcns.c $(COMLIST)/fcns.c \
D 2
	$(COMINC)/filehdr.h $(COMINC)/ldfcn.h $(M32INC)/sgs.h \
E 2
I 2
	$(COMINC)/filehdr.h $(COMINC)/ldfcn.h $(M68KINC)/sgs.h \
E 2
	$(COMINC)/linenum.h $(COMINC)/syms.h $(COMINC)/storclass.h \
	$(COMLIST)/defs.h $(COMLIST)/findname.h \
D 2
	$(M32INC)/paths.h
E 2
I 2
	$(M68KINC)/paths.h
E 2
	$(LINT_CMD) $(DEFLIST) $(INCLIST) $(COMLIST)/*.c
E 1
