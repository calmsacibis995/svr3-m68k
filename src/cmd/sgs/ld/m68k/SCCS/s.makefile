h01682
s 00004/00004/00340
d D 1.4 86/09/17 16:26:04 fnf 4 3
c Changes for release integration.
e
s 00000/00000/00344
d D 1.3 86/08/18 08:46:00 fnf 3 2
c Checkpoint changes at first working shared library version
e
s 00030/00028/00314
d D 1.2 86/07/30 14:27:42 fnf 2 1
c Add macro based C debugging package macros.
e
s 00342/00000/00000
d D 1.1 86/07/29 14:33:34 fnf 1 0
c Baseline code from 5.3 release for 3b2
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
#ident	"@(#)ld:m32/makefile	1.25"
E 2
I 2
#ident	"@(#)ld:m68k/makefile	1.25"
E 2
#
D 2
#	BELLMAC-32 LD MASTER MAKEFILE
E 2
I 2
#	MOTOROLA M68K LD MASTER MAKEFILE
E 2
#
#
# Architectures:
#
#	AR16WR = DEC 11/45-70
#	AR32WR = VAX 11/780
D 2
#	AR32W  = 3B family, IBM 370
E 2
I 2
#	AR32W  = 3B family, IBM 370, Motorola M68K
E 2
#
# Specify the version of UN*X beng used:
#
#	TS  :	BTL UN*X TS
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
B10=
SYS=TS
FFLAG=
I 4
DBUG=-DNO_DBUG
E 4
#	Set ARFORMAT to PORTAR for portable archive format (post 5.0)
#	Set ARFORMAT to PORT5AR for 5.0 archive format
#	Set ARFORMAT to OLDAR for pre-5.0 archive format
ARFORMAT=PORTAR
DBO=FBO
ENV=
FRC=
FLEX= -DFLEXNAMES
D 4
DEFLIST= -D$(ARCH) -D$(ARFORMAT) -D$(DBO) $(FLEX)
E 4
I 4
DEFLIST= -D$(ARCH) -D$(ARFORMAT) -D$(DBO) $(FLEX) $(DBUG)
E 4
D 2
INCLIST= -I. -I$(COMLD) -I$(M32INC) -I$(COMINC) -I$(N3BINC) -I$(USRINC)
E 2
I 2
INCLIST= -I. -I$(COMLD) -I$(M68KINC) -I$(COMINC) -I$(N3BINC) -I$(USRINC)
E 2
CC_CMD=$(CC) $(CFLAGS) $(DEFLIST) $(INCLIST) $(ENV)
#
LDFLAGS=
LDLIBS=
LD_CMD=$(CC) $(LDFLAGS)
#
YACC=yacc
YFLAGS=-d
YACC_CMD=$(YACC) $(YFLAGS)
#
LEX_CMD=lex
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
M32INC= $(BASE)/inc/m32
E 2
I 2
BINDIR= $(BASE)/xenv/m68k/bin
M68KINC= $(BASE)/inc/m68k
E 2
COMLD= $(BASE)/ld/common
COMINC= $(BASE)/inc/common
D 2
N3BINC= $(BASE)/inc/3b
E 2
I 2
#N3BINC= $(BASE)/inc/3b
E 2
#
#	External Directories
#
ROOT=
VERSION=
D 2
LIBLD	= $(BASE)/libld/m32/libld.a
E 2
I 2
D 4
#(fnf) LIBLD	= $(BASE)/libld/m68k/libld.a
LIBLD	= /usr/lib/libld.a
E 4
I 4
LIBLD	= $(BASE)/libld/m68k/libld.a
E 4
E 2
USRINC= $(ROOT)/usr/$(VERSION)include
I 2
D 4
LIBDBUG = /usr/local/lib/libdbug.a
E 4
I 4
#LIBDBUG = /usr/local/lib/libdbug.a
E 4
E 2
#
D 2
LIBES= -ll -ly 
E 2
I 2
LIBES= -ll -ly $(LIBDBUG)
E 2
LIBS=
#
#
# Specify the *.c files making up ld:
#
CFILES=$(COMLD)/alloc.c $(COMLD)/expr0.c $(COMLD)/expr1.c \
	$(COMLD)/globs.c $(COMLD)/instr.c \
	$(COMLD)/ld00.c $(COMLD)/ld01.c $(COMLD)/ld1.c $(COMLD)/ld2.c \
	$(COMLD)/lists.c $(COMLD)/output.c $(COMLD)/syms.c \
	$(COMLD)/slotvec.c $(COMLD)/tv.c $(COMLD)/util.c \
	$(COMLD)/version.c \
	lex.yy.c $(COMLD)/maps.c y.tab.c sglobs.c special1.c special2.c specsyms.c
#
#
# Specify the *.h files making up ld:
#
HFILES1=y.tab.h params.h system.h \
	$(COMLD)/ldmacros.h $(COMLD)/attributes.h $(COMLD)/list.h \
	$(COMLD)/structs.h $(COMLD)/extrns.h $(COMLD)/ldtv.h \
	$(COMINC)/aouthdr.h $(COMLD)/slotvec.h \
	$(COMLD)/instr.h
HFILES2=$(COMINC)/filehdr.h $(COMINC)/scnhdr.h $(COMINC)/syms.h \
	$(COMINC)/storclass.h $(COMINC)/bool.h \
	$(COMINC)/sgsmacros.h $(COMINC)/ldfcn.h $(COMINC)/reloc.h \
	$(COMINC)/linenum.h $(COMINC)/patch.h
D 2
HFILES3=$(M32INC)/sgs.h $(M32INC)/paths.h $(M32INC)/tv.h $(N3BINC)/magic.h
E 2
I 2
HFILES3=$(M68KINC)/sgs.h $(M68KINC)/paths.h $(M68KINC)/tv.h $(M68KINC)/magic.h
E 2
#
# Specify the *.o files making up ld:
#
O	= o
OBJECTS=expr0.$O globs.$O sglobs.$O ld00.$O lex.yy.$O lists.$O \
	util.$O version.$O y.tab.$O special1.$O special2.$O \
	alloc.$O specsyms.$O expr1.$O ld01.$O ld1.$O ld2.$O maps.$O \
	output.$O slotvec.$O syms.$O tv.$O instr.$O
#
#
# Make ld:
#
all:		ld
#
ld:		$(OBJECTS) $(LIBLD)
	$(LD_CMD) -o ld $(OBJECTS) $(LIBES) $(LIBLD) $(LIBS) $(LDLIBS)
#
#
# Make the *.c:
#
y.tab.c y.tab.h:	$(COMLD)/ld.yac $(FRC)
	@echo "Expect to see message 'conflicts: 7 shift/reduce'";
	$(YACC_CMD) $(COMLD)/ld.yac
#
y.tab.$O:	$(COMLD)/attributes.h $(COMLD)/list.h $(COMLD)/structs.h $(FRC) \
		$(COMLD)/ldmacros.h $(COMLD)/extrns.h $(COMLD)/ldtv.h system.h \
		$(COMINC)/filehdr.h $(COMINC)/scnhdr.h $(COMINC)/syms.h \
		$(COMINC)/storclass.h $(COMINC)/bool.h $(COMINC)/sgsmacros.h \
D 2
		$(M32INC)/sgs.h $(M32INC)/tv.h \
E 2
I 2
		$(M68KINC)/sgs.h $(M68KINC)/tv.h \
E 2
		params.h \
		y.tab.c
	$(CC_CMD) y.tab.c
#
lex.yy.c:	$(COMLD)/ld.lex $(FRC)
	$(LEX_CMD) $(COMLD)/ld.lex
#
lex.yy.$O:	$(COMLD)/structs.h $(COMLD)/extrns.h system.h $(FRC) \
		$(COMLD)/ldtv.h \
		$(COMLD)/instr.h $(COMINC)/aouthdr.h \
		$(COMINC)/filehdr.h $(COMINC)/scnhdr.h $(COMINC)/syms.h \
		$(COMINC)/storclass.h $(COMINC)/bool.h \
D 2
		$(M32INC)/paths.h $(M32INC)/sgs.h $(M32INC)/tv.h \
E 2
I 2
		$(M68KINC)/paths.h $(M68KINC)/sgs.h $(M68KINC)/tv.h \
E 2
		y.tab.h \
		lex.yy.c
	$(CC_CMD) lex.yy.c
#
expr0.$O:	$(COMLD)/extrns.h $(COMLD)/structs.h system.h $(FRC) \
		$(COMLD)/ldmacros.h \
		$(COMINC)/filehdr.h $(COMINC)/scnhdr.h $(COMINC)/syms.h \
		$(COMINC)/storclass.h $(COMINC)/sgsmacros.h \
		y.tab.h \
		$(COMLD)/expr0.c
	$(CC_CMD) $(COMLD)/expr0.c
#
expr1.$O:	$(COMLD)/extrns.h $(COMLD)/structs.h system.h $(FRC) \
		$(COMLD)/ldmacros.h \
		$(COMINC)/filehdr.h $(COMINC)/scnhdr.h $(COMINC)/syms.h \
		$(COMINC)/storclass.h $(COMINC)/sgsmacros.h \
		y.tab.h \
		$(COMLD)/expr1.c
	$(CC_CMD) $(COMLD)/expr1.c
#
ld00.$O:	$(COMLD)/extrns.h $(COMLD)/list.h $(COMLD)/structs.h $(FRC) \
		system.h $(COMINC)/ldfcn.h \
		$(COMINC)/filehdr.h $(COMINC)/scnhdr.h $(COMINC)/syms.h \
		$(COMINC)/storclass.h $(COMINC)/sgsmacros.h \
D 2
		$(M32INC)/paths.h $(M32INC)/sgs.h \
E 2
I 2
		$(M68KINC)/paths.h $(M68KINC)/sgs.h \
E 2
		params.h y.tab.h \
		$(COMLD)/ld00.c
	$(CC_CMD) $(COMLD)/ld00.c
#
ld01.$O:	$(COMLD)/extrns.h $(COMLD)/structs.h $(COMLD)/list.h $(FRC) \
		$(COMLD)/ldmacros.h system.h $(COMLD)/ldtv.h $(COMLD)/instr.h \
		$(COMINC)/filehdr.h $(COMINC)/scnhdr.h $(COMINC)/syms.h \
		$(COMINC)/storclass.h $(COMINC)/sgsmacros.h \
D 2
		$(M32INC)/tv.h \
E 2
I 2
		$(M68KINC)/tv.h \
E 2
		$(COMLD)/ld01.c
	$(CC_CMD) $(COMLD)/ld01.c
#
ld1.$O:		$(COMLD)/extrns.h $(COMLD)/list.h $(COMLD)/ldtv.h $(FRC) \
		$(COMLD)/ldmacros.h \
		$(COMLD)/structs.h system.h $(COMINC)/aouthdr.h \
		$(COMINC)/filehdr.h $(COMINC)/scnhdr.h $(COMINC)/syms.h \
		$(COMINC)/storclass.h $(COMINC)/ldfcn.h $(COMINC)/reloc.h \
		$(COMINC)/sgsmacros.h \
D 2
		$(M32INC)/sgs.h $(M32INC)/tv.h $(N3BINC)/magic.h \
E 2
I 2
		$(M68KINC)/sgs.h $(M68KINC)/tv.h $(M68KINC)/magic.h \
E 2
		params.h \
		$(COMLD)/instr.h \
		$(COMLD)/ld1.c
	$(CC_CMD) $(COMLD)/ld1.c
#
ld2.$O:		$(COMLD)/ldtv.h $(COMLD)/extrns.h $(COMLD)/list.h $(FRC) \
		$(COMLD)/structs.h system.h \
		$(COMINC)/filehdr.h $(COMINC)/scnhdr.h $(COMINC)/syms.h \
		$(COMINC)/sgsmacros.h $(COMINC)/storclass.h \
		$(COMINC)/reloc.h $(COMINC)/linenum.h $(COMINC)/patch.h \
D 2
		$(M32INC)/tv.h \
E 2
I 2
		$(M68KINC)/tv.h \
E 2
		y.tab.h params.h \
		$(COMLD)/instr.h \
		$(COMLD)/ld2.c
	$(CC_CMD) $(COMLD)/ld2.c
#
alloc.$O:	$(COMLD)/extrns.h $(COMLD)/list.h $(COMLD)/structs.h $(FRC) \
		$(COMLD)/ldmacros.h system.h $(COMLD)/ldtv.h \
		$(COMINC)/filehdr.h $(COMINC)/scnhdr.h $(COMINC)/syms.h \
		$(COMINC)/storclass.h $(COMINC)/sgsmacros.h \
D 2
		$(M32INC)/tv.h \
E 2
I 2
		$(M68KINC)/tv.h \
E 2
		params.h \
		$(COMLD)/alloc.c
	$(CC_CMD) $(COMLD)/alloc.c
#
output.$O:	$(COMLD)/structs.h $(COMLD)/extrns.h system.h $(FRC) \
		$(COMLD)/ldtv.h $(COMLD)/slotvec.h $(COMINC)/aouthdr.h \
		$(COMLD)/ldmacros.h $(COMLD)/instr.h \
		$(COMINC)/filehdr.h $(COMINC)/scnhdr.h $(COMINC)/syms.h \
		$(COMINC)/sgsmacros.h $(COMINC)/storclass.h \
		$(COMINC)/reloc.h $(COMINC)/linenum.h $(COMINC)/ldfcn.h \
		$(COMINC)/patch.h \
D 2
		$(M32INC)/sgs.h $(M32INC)/tv.h \
E 2
I 2
		$(M68KINC)/sgs.h $(M68KINC)/tv.h \
E 2
		params.h \
		$(COMLD)/output.c
	$(CC_CMD) $(COMLD)/output.c
#
maps.$O:	$(COMLD)/extrns.h $(COMLD)/structs.h $(COMLD)/attributes.h $(FRC) \
		system.h \
		$(COMINC)/filehdr.h $(COMINC)/scnhdr.h $(COMINC)/syms.h \
		$(COMINC)/storclass.h $(COMINC)/sgsmacros.h \
		$(COMLD)/maps.c
	$(CC_CMD) $(COMLD)/maps.c
#
util.$O:	$(COMLD)/structs.h $(COMLD)/extrns.h system.h $(FRC) \
		$(COMINC)/filehdr.h $(COMINC)/scnhdr.h $(COMINC)/syms.h \
		$(COMINC)/storclass.h $(COMINC)/sgsmacros.h \
D 2
		$(COMLD)/slotvec.h $(COMINC)/reloc.h $(M32INC)/sgs.h \
E 2
I 2
		$(COMLD)/slotvec.h $(COMINC)/reloc.h $(M68KINC)/sgs.h \
E 2
		y.tab.h \
		$(COMLD)/util.c
	$(CC_CMD) $(COMLD)/util.c
#
lists.$O: 	$(COMLD)/list.h $(COMLD)/structs.h system.h $(FRC) \
		$(COMINC)/filehdr.h $(COMINC)/scnhdr.h $(COMINC)/syms.h \
		$(COMINC)/sgsmacros.h $(COMINC)/storclass.h \
		$(COMLD)/lists.c
	$(CC_CMD) $(COMLD)/lists.c
#
syms.$O:	$(COMLD)/structs.h $(COMLD)/extrns.h $(COMLD)/ldtv.h $(FRC) \
		system.h $(COMLD)/instr.h \
		$(COMLD)/ldmacros.h $(COMINC)/ldfcn.h \
		$(COMINC)/filehdr.h $(COMINC)/scnhdr.h $(COMINC)/syms.h \
		$(COMINC)/storclass.h $(COMINC)/sgsmacros.h \
D 2
		$(M32INC)/paths.h $(M32INC)/tv.h \
E 2
I 2
		$(M68KINC)/paths.h $(M68KINC)/tv.h \
E 2
		$(COMLD)/syms.c
	$(CC_CMD) $(COMLD)/syms.c
#
tv.$O:		$(COMLD)/extrns.h $(COMLD)/structs.h $(COMLD)/ldtv.h $(FRC) \
		$(COMLD)/ldmacros.h system.h $(COMLD)/list.h \
		$(COMINC)/filehdr.h $(COMINC)/scnhdr.h $(COMINC)/syms.h \
		$(COMINC)/bool.h $(COMINC)/storclass.h \
		$(COMINC)/reloc.h $(COMINC)/ldfcn.h $(COMINC)/sgsmacros.h \
D 2
		$(M32INC)/tv.h \
E 2
I 2
		$(M68KINC)/tv.h \
E 2
		$(COMLD)/tv.c
	$(CC_CMD) $(COMLD)/tv.c
#
globs.$O:	$(COMLD)/structs.h $(COMLD)/ldtv.h system.h $(FRC) \
		$(COMINC)/filehdr.h $(COMINC)/scnhdr.h $(COMINC)/syms.h \
		$(COMINC)/storclass.h \
D 2
		$(M32INC)/paths.h $(M32INC)/tv.h \
E 2
I 2
		$(M68KINC)/paths.h $(M68KINC)/tv.h \
E 2
		params.h \
		$(COMLD)/globs.c
	$(CC_CMD) $(COMLD)/globs.c
#
instr.$O:	$(COMLD)/instr.c $(FRC)
	$(CC_CMD) $(COMLD)/instr.c
#
slotvec.$O:	$(COMLD)/slotvec.h $(COMLD)/structs.h $(COMLD)/extrns.h $(FRC) \
		$(COMINC)/filehdr.h $(COMINC)/scnhdr.h $(COMINC)/syms.h \
		$(COMINC)/storclass.h \
D 2
       		system.h $(M32INC)/paths.h \
E 2
I 2
       		system.h $(M68KINC)/paths.h \
E 2
		$(COMLD)/slotvec.c
	$(CC_CMD) $(COMLD)/slotvec.c
#
version.$O:	$(COMLD)/version.c $(FRC)
	$(CC_CMD) $(COMLD)/version.c
#
sglobs.$O:	$(COMLD)/structs.h $(COMINC)/filehdr.h $(FRC) \
		$(COMINC)/scnhdr.h $(COMINC)/syms.h $(COMINC)/storclass.h \
D 2
		$(M32INC)/sgs.h $(M32INC)/tv.h $(COMLD)/ldtv.h \
E 2
I 2
		$(M68KINC)/sgs.h $(M68KINC)/tv.h $(COMLD)/ldtv.h \
E 2
		system.h sglobs.c
	$(CC_CMD) sglobs.c
#
special1.$O:	system.h special1.c $(COMLD)/structs.h $(COMLD)/extrns.h $(FRC)  \
		$(COMINC)/filehdr.h $(COMINC)/scnhdr.h $(COMINC)/syms.h $(COMINC)/storclass.h
	$(CC_CMD) special1.c
#
special2.$O:	$(COMINC)/aouthdr.h $(COMLD)/structs.h $(COMINC)/filehdr.h $(FRC)  \
		$(COMINC)/scnhdr.h $(COMINC)/syms.h $(COMINC)/storclass.h $(COMINC)/ldfcn.h \
D 2
		$(M32INC)/tv.h $(COMLD)/ldtv.h $(COMLD)/slotvec.h \
E 2
I 2
		$(M68KINC)/tv.h $(COMLD)/ldtv.h $(COMLD)/slotvec.h \
E 2
		$(COMINC)/reloc.h $(COMLD)/extrns.h $(COMLD)/list.h \
D 2
		$(M32INC)/sgs.h $(N3BINC)/magic.h  $(COMINC)/sgsmacros.h \
E 2
I 2
		$(M68KINC)/sgs.h $(M68KINC)/magic.h  $(COMINC)/sgsmacros.h \
E 2
		system.h special2.c
	$(CC_CMD) special2.c
#
specsyms.$O:	system.h $(COMLD)/extrns.h $(COMINC)/sgsmacros.h $(FRC)  \
		$(COMLD)/structs.h $(COMINC)/filehdr.h $(COMINC)/scnhdr.h $(COMINC)/syms.h $(COMINC)/storclass.h \
D 2
		$(M32INC)/tv.h $(COMLD)/ldtv.h \
E 2
I 2
		$(M68KINC)/tv.h $(COMLD)/ldtv.h \
E 2
		specsyms.c
	$(CC_CMD) specsyms.c
#
#
#
# File and administrative commands:
#
install:	$(BINDIR)/$(SGS)ld
#
$(BINDIR)/$(SGS)ld:	ld
	cp ld  ld.bak
	-$(STRIP_CMD) ld
	sh $(BASE)/sgs.install 755 $(OWN) $(GRP) $(BINDIR)/$(SGS)ld ld
	mv ld.bak ld
#
save:	$(BINDIR)/$(SGS)ld
	-rm -f $(BINDIR)/$(SGS)ld.back
	cp $(BINDIR)/$(SGS)ld   $(BINDIR)/$(SGS)ld.back
#
uninstall:	$(BINDIR)/$(SGS)ld.back
	-rm -f $(BINDIR)/$(SGS)ld
	cp  $(BINDIR)/$(SGS)ld.back    $(BINDIR)/$(SGS)ld
#
shrink:
	-rm -f $(OBJECTS) y.tab.c lex.yy.c y.tab.h ld
#
# Run lint on the *.c files:
#
lint:		$(CFILES) $(HFILES1) $(HFILES2) $(HFILES3)
	$(LINT_CMD) $(DEFLIST) $(INCLIST) $(CFILES)
#
# Force recompilation
#
FRC:
E 1
