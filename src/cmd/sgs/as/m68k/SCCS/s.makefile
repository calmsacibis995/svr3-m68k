h31793
s 00001/00001/00203
d D 1.10 86/11/21 12:08:49 fnf 10 9
c Remove default of -g for SDBFLAG.
e
s 00000/00000/00204
d D 1.9 86/09/17 16:22:43 fnf 9 8
c Changes for release integration.
e
s 00000/00000/00204
d D 1.8 86/09/16 15:25:23 fnf 8 7
c Changes to make ident directive work.
e
s 00005/00002/00199
d D 1.7 86/09/15 14:00:10 fnf 7 6
c Changes to remove multisegment capability for "init" and "lib" sections.
e
s 00001/00001/00200
d D 1.6 86/09/11 14:37:25 fnf 6 5
c Pick up headers from inc/common.
e
s 00000/00000/00201
d D 1.5 86/09/11 12:59:57 fnf 5 4
c Finish work for shared libraries, checkpoint changes.
e
s 00000/00000/00201
d D 1.4 86/08/18 08:47:41 fnf 4 3
c Checkpoint changes at first working shared library version
e
s 00003/00002/00198
d D 1.3 86/08/07 08:05:31 fnf 3 2
c Checkpoint shared library changes so far.  Still does not work
c but accepts "init" and "lib" section directives without crashing...
e
s 00003/00002/00197
d D 1.2 86/08/04 15:46:43 fnf 2 1
c Initial changes to support "init" and "lib" sections for
c shared libraries.
e
s 00199/00000/00000
d D 1.1 86/08/04 09:39:28 fnf 1 0
c Start with baseline 5.2 68000 assembler.
e
u
U
t
T
I 1
#	@(#)makefile	6.3	
#
#	m68k AS MAKEFILE
#	     generic makefile for mc68000 family
#	     The environment/makefile variable PROCESSOR
#	     should be set to indicate which one.
#	     Current values are M68000, M68010, M68020
#
#	

SGS=
OWN=	bin		#file owner
GRP=	bin		#file group
PROT=	755		#protections
ROOT=	

BINDIR=	$(ROOT)/bin
BASE = ../..
M68INC=	$(BASE)/inc/m68k
D 6
COMINC=	/usr/include
E 6
I 6
COMINC=	$(BASE)/inc/common
E 6
COMAS=	$(BASE)/as/common

ARCH=	AR32W
FLEX=	-DFLEXNAMES
I 2
D 10
SDBFLAG=	-g
E 10
I 10
SDBFLAG=
E 10
E 2

COPROCESSOR = -DM68881

CC=	cc
LINT=	lint
YACC=	yacc
LFLAGS=	-p
D 2
CFLAGS=	-O -D$(PROCESSOR) $(COPROCESSOR)
E 2
I 2
CFLAGS=	-O -D$(PROCESSOR) $(COPROCESSOR) $(SDBFLAG)
E 2
INCLIST=	-I. -I./$(PROCESSOR) -I$(M68INC) -I$(COMINC) -I$(COMAS)
D 7
DEFLIST=	-DINT32BIT
E 7
I 7
DEFLIST=	-DINT32BIT $(DBUG)
E 7
ODEFLIST=	-DONEPROC $(FLEX) -D$(ARCH)
D 2
LDFLAGS=
E 2
I 2
LDFLAGS=	$(SDBFLAG)
E 2
YFLAGS=		-d
I 3
D 7
LIBS=		/usr/local/lib/libdbug.a
E 7
I 7
#DBUG=
DBUG=		-DNO_DBUG
#LIBS=		/usr/local/lib/libdbug.a
LIBS=
E 7
E 3

CC_CMD=	$(CC) -c $(CFLAGS) $(INCLIST) $(DEFLIST) $(ODEFLIST)
YACC_CMD=	$(YACC) $(YFLAGS)


OFILES=	pass0.o instab.o parse.o gencode.o pass1.o symbols.o \
	code.o expand1.o expand2.o errors.o addr2.o pass2.o  \
	addr1.o codeout.o getstab.o obj.o symlist.o dofpnum.o

CFILES=	$(COMAS)/pass0.c $(COMAS)/instab.c parse.c 	     \
	$(PROCESSOR)/gencode.c $(PROCESSOR)/addr2.c	     \
	$(PROCESSOR)/expand2.c 			     \
	$(COMAS)/pass1.c $(COMAS)/symbols.c $(COMAS)/code.c  \
	$(COMAS)/expand1.c $(COMAS)/errors.c		     \
	$(COMAS)/pass2.c $(COMAS)/addr1.c		     \
	$(COMAS)/codeout.c $(COMAS)/getstab.c $(COMAS)/obj.c \
	$(COMAS/symlist.c \
	dofpnum.c

HFILES=	$(COMAS)/codeout.h gendefs.h 			     \
	$(PROCESSOR)/instab.h $(PROCESSOR)/ops.out	     \
	$(COMAS)/symbols.h $(M68INC)/paths.h		     \
	$(COMAS)/expand.h expand2.h parse.h		     \
	$(COMINC)/reloc.h $(COMINC)/syms.h		     \
	$(COMINC)/storclass.h $(COMINC)/filehdr.h	     \
	$(COMINC)/scnhdr.h $(COMINC)/linenum.h		     \
	$(M68INC)/sgs.h	\
	fpnum.h fpinit.h

build:	$(SGS)as

#-------------------------
$(SGS)as:	$(OFILES)
D 3
		$(CC) $(CFLAGS) $(LDFLAGS) -o $(SGS)as $(OFILES)
E 3
I 3
		$(CC) $(CFLAGS) $(LDFLAGS) -o $(SGS)as $(OFILES) $(LIBS);
E 3

pass0.o:	$(COMAS)/pass0.c pass0.h $(M68INC)/paths.h   \
D 3
		$(M68INC)/sgs.h systems.h
E 3
I 3
		$(M68INC)/sgs.h systems.h gendefs.h
E 3
		$(CC_CMD) $(COMAS)/pass0.c

# The baroque code here tries to keep file times in sync to
# avoid unnecessary re-compilations.

parse.o:	parse.c $(COMAS)/symbols.h $(PROCESSOR)/instab.h gendefs.h
		$(CC_CMD) parse.c

instab.o:	$(COMAS)/instab.c $(PROCESSOR)/instab.h $(PROCESSOR)/ops.out parse.h \
		$(COMAS)/symbols.h systems.h
		$(CC_CMD) $(COMAS)/instab.c

parse.c:	y.tab.c
		if cmp -s y.tab.c parse.c;then :;else cp y.tab.c parse.c;fi
parse.h:	y.tab.h
		if cmp -s y.tab.h parse.h;then :;else cp y.tab.h parse.h;fi

y.tab.c y.tab.h:	$(PROCESSOR)/parse.y
		$(YACC_CMD) $(PROCESSOR)/parse.y

# end baroque code

code.o:		$(COMAS)/code.c $(COMAS)/symbols.h	     \
		$(COMAS)/codeout.h gendefs.h systems.h
		$(CC_CMD) $(COMAS)/code.c

errors.o:	$(COMAS)/errors.c gendefs.h systems.h
		$(CC_CMD) $(COMAS)/errors.c

pass1.o:	$(COMAS)/pass1.c $(M68INC)/paths.h gendefs.h \
		$(COMAS)/symbols.h systems.h
		$(CC_CMD) $(COMAS)/pass1.c

gencode.o:	$(PROCESSOR)/gencode.c $(COMAS)/symbols.h $(PROCESSOR)/instab.h\
		gendefs.h $(COMAS)/expand.h expand2.h
		$(CC_CMD) $(PROCESSOR)/gencode.c

expand1.o:	$(COMAS)/expand1.c $(COMAS)/expand.h	     \
		$(COMAS)/symbols.h gendefs.h systems.h
		$(CC_CMD) $(COMAS)/expand1.c

expand2.o:	$(PROCESSOR)/expand2.c $(COMAS)/expand.h expand2.h	     \
		$(COMAS)/symbols.h
		$(CC_CMD) $(PROCESSOR)/expand2.c

addr1.o:	$(COMAS)/addr1.c $(COMINC)/reloc.h gendefs.h \
		$(COMINC)/syms.h $(COMINC)/storclass.h	     \
		$(COMINC)/linenum.h $(COMINC)/filehdr.h	     \
		$(COMAS)/symbols.h $(COMAS)/codeout.h	     \
		systems.h
		$(CC_CMD) $(COMAS)/addr1.c

addr2.o:	$(PROCESSOR)/addr2.c $(COMINC)/reloc.h $(COMAS)/symbols.h \
		$(PROCESSOR)/instab.h gendefs.h $(COMAS)/codeout.h
		$(CC_CMD) $(PROCESSOR)/addr2.c

codeout.o:	$(COMAS)/codeout.c $(COMAS)/symbols.h	     \
		$(COMAS)/codeout.h gendefs.h systems.h
		$(CC_CMD) $(COMAS)/codeout.c

getstab.o:	$(COMAS)/getstab.c $(COMAS)/symbols.h	     \
		gendefs.h systems.h
		$(CC_CMD) $(COMAS)/getstab.c

pass2.o:	$(COMAS)/pass2.c gendefs.h systems.h	     \
		$(COMAS)/symbols.h
		$(CC_CMD) $(COMAS)/pass2.c

obj.o:		$(COMAS)/obj.c $(COMINC)/filehdr.h	     \
		$(COMINC)/linenum.h $(COMINC)/reloc.h	     \
		$(COMINC)/scnhdr.h $(COMINC)/syms.h	     \
		$(COMINC)/storclass.h $(COMAS)/symbols.h     \
		$(COMAS)/codeout.h gendefs.h systems.h	     \
		$(M68INC)/sgs.h
		$(CC_CMD) $(COMAS)/obj.c

symlist.o:	$(COMAS)/symlist.c $(COMAS)/symbols.h	     \
		$(COMINC)/syms.h $(COMINC)/storclass.h	     \
		gendefs.h systems.h
		$(CC_CMD) $(COMAS)/symlist.c

symbols.o:	$(COMAS)/symbols.c $(COMAS)/symbols.h	     \
		symbols2.h systems.h
		$(CC_CMD) $(COMAS)/symbols.c

dofpnum.o:	dofpnum.c fpnum.h fpinit.h
		$(CC_CMD) dofpnum.c

#-------------------------

install:	$(SGS)as
		-rm -f $(BINDIR)/$(SGS)as
		cp $(SGS)as $(BINDIR)
		strip $(BINDIR)/$(SGS)as
		chmod $(PROT) $(BINDIR)/$(SGS)as
		chgrp $(GRP) $(BINDIR)/$(SGS)as
		chown $(OWN) $(BINDIR)/$(SGS)as

#--------------------------

save:	$(BINDIR)/$(SGS)as
	-rm -f $(BINDIR)/$(SGS)as.back
	cp $(BINDIR)/$(SGS)as $(BINDIR)/$(SGS)as.back

#--------------------------

uninstall:	$(BINDIR)/$(SGS)as.back
		-rm -f $(BINDIR)/$(SGS)as
		cp $(BINDIR)/$(SGS)as.back $(BINDIR)/$(SGS)as

#--------------------------

clean:
	-rm -f $(OFILES)

#--------------------------

clobber:	clean
		-rm -f $(SGS)as parse.c parse.h y.tab.c y.tab.h

#--------------------------

lint:	$(COMINC)/a.out.h $(COMINC)/ar.h \
	$(CFILES)
	$(LINT) $(LFLAGS) -I$(M68INC) -I$(COMINC) $(CFILES) >lint.out
E 1
