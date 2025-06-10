#	@(#)makefile	6.5	
#	M68000/010/020 DISASSEMBLER MAKEFILE
#
#	@(#) makefile:	2.1 83/07/08
#
#
SGS=
OWN=	sgs		#file owner
GRP=	sgs		#file group
PROT=	775		#protections
ROOT=	$(OL)
BASE=	../../..

OL = 
BIN = $(OL)/bin
TESTDIR= .
INCM68K= ../m68k
M68INC=	$(BASE)/sgs/inc/m68k
COMSRC=	../common

LIBLD = -lld

INCLIST=	-I$(COMSRC) -I$(M68INC) -I$(INCM68K) -I$(PROCESSOR)

CC=	cc
LINT=	lint
LFLAGS=	-p 
CFLAGS=	-O
LDFLAGS= -s
DEFLIST=-D$(PROCESSOR)

ARCH=	AR32W

FLEX=	-DFLEXNAMES
ARFORMAT=	PORTAR

# Other definitions

ODEFLIST=	-D$(ARFORMAT) -DUNIX=4 -DMC68 -D$(ARCH) $(FLEX)

CC_CMD=	$(CC) -c $(CFLAGS) $(DEFLIST) $(ODEFLIST) $(INCLIST)


OFILES=	dis_extn.o dis_main.o dis_utls.o text_dis.o lists.o extended.o names.o\
	cp_gen.o 881_gen.o
CFILES=	$(COMSRC)/dis_extn.c $(COMSRC)/dis_main.c $(COMSRC)/dis_utls.c \
	  $(COMSRC)/lists.c \
	  $(PROCESSOR)/text_dis.c $(PROCESSOR)/extended.c $(PROCESSOR)/names.c\
	  $(PROCESSOR)/cp_gen.c $(PROCESSOR)/881_gen.c


#-------------------------

$(TESTDIR)/dis:	$(OFILES)
		$(CC) $(CFLAGS) $(LDFLAGS) -o $(TESTDIR)/dis $(OFILES) $(LIBLD)

dis_extn.o:	dis.h \
		$(COMSRC)/structs.h \
		$(COMSRC)/dis_extn.c 
		$(CC_CMD) $(COMSRC)/dis_extn.c

lists.o:	dis.h \
		$(COMSRC)/lists.c \
		$(COMSRC)/structs.h \
		$(M68INC)/paths.h \
		$(M68INC)/sgs.h 
		$(CC_CMD) $(COMSRC)/lists.c

dis_main.o:	dis.h \
		$(COMSRC)/structs.h \
		$(COMSRC)/dis_main.c \
		$(M68INC)/paths.h \
		$(M68INC)/sgs.h 
		$(CC_CMD) $(COMSRC)/dis_main.c

dis_utls.o:	dis.h \
		$(COMSRC)/structs.h \
		$(COMSRC)/dis_utls.c \
		$(M68INC)/sgs.h
		$(CC_CMD) $(COMSRC)/dis_utls.c

text_dis.o:	dis.h \
		$(PROCESSOR)/text_dis.c \
		$(COMSRC)/structs.h \
		$(M68INC)/sgs.h 
		$(CC_CMD) $(PROCESSOR)/text_dis.c

extended.o:	dis.h $(PROCESSOR)/names.h \
		$(PROCESSOR)/extended.c
		$(CC_CMD) $(PROCESSOR)/extended.c

names.o:	dis.h $(PROCESSOR)/names.h \
		$(PROCESSOR)/names.c
		$(CC_CMD) $(PROCESSOR)/names.c

cp_gen.o:	dis.h $(PROCESSOR)/names.h $(PROCESSOR)/cp_gen.c
		$(CC_CMD) $(PROCESSOR)/cp_gen.c

881_gen.o:	dis.h $(PROCESSOR)/names.h $(PROCESSOR)/881_gen.c
		$(CC_CMD) $(PROCESSOR)/881_gen.c

#-------------------------

all compile build:$(TESTDIR)/dis $(OFILES)

#-------------------------

install:	all
		cp $(TESTDIR)/dis $(BIN)/dis
		strip $(BIN)/dis
		chmod $(PROT) $(BIN)/dis
		chgrp $(GRP) $(BIN)/dis
		chown $(OWN) $(BIN)/dis

#--------------------------

clean:
	-rm -f $(OFILES)

#--------------------------

clobber shrink:	clean
		-rm -f $(TESTDIR)/dis

