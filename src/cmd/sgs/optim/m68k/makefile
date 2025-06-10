#	@(#)makefile	7.2	
#	MC68000 OPTIM MAKEFILE
#
#	@(#) makefile:	2.1 83/07/08
#
OWN=	bin		#file owner
GRP=	bin		#file group
PROT=	775		#protections
ROOT=	

BINDIR=	$(ROOT)/bin
LIBDIR=	$(ROOT)/lib
COMDIR= ../common

CC=	cc
LINT=	lint
LFLAGS=	-p
CFLAGS=	-O
LDFLAGS=
#FPU=NONE
FPU=M68881
DEFLIST=	-D$(PROCESSOR) -D$(FPU)
AWKLIST=	p=$(PROCESSOR)
INCLIST=	-I./$(PROCESSOR) -I. -I$(COMDIR)

CC_CMD=	$(CC) -c $(CFLAGS) $(DEFLIST) $(INCLIST)


OFILES=		scan.o parse.o optab.o rand.o mipsup.o vtrace.o \
		optim.o inter.o
CFILES=		$(PROCESSOR)/scan.c $(PROCESSOR)/mipsup.c \
		$(PROCESSOR)/vtrace.c $(PROCESSOR)/optab.c \
		parse.c rand.c \
		$(COMDIR)/optim.c $(COMDIR)/inter.c
OP_FILES=	$(PROCESSOR)/ops.tail $(PROCESSOR)/optab.tail $(PROCESSOR)/ops.h

OPTIM_H=	$(COMDIR)/optim.h defs $(PROCESSOR)/rand.h $(PROCESSOR)/ops.h

build:	optim

#-------------------------
optim:	$(OFILES)
		$(CC) $(CFLAGS) $(LDFLAGS) -o optim $(OFILES)

optim.o:	$(OPTIM_H) $(COMDIR)/optim.c
		$(CC_CMD) $(COMDIR)/optim.c

inter.o:	$(OPTIM_H) $(COMDIR)/inter.c
		$(CC_CMD) $(COMDIR)/inter.c

scan.o:		$(OPTIM_H) $(PROCESSOR)/scan.c
		$(CC_CMD) $(PROCESSOR)/scan.c

parse.o:	$(OPTIM_H) parse.c
		$(CC_CMD) parse.c

rand.o:		$(OPTIM_H) rand.c
		$(CC_CMD) rand.c

mipsup.o:	$(OPTIM_H) $(PROCESSOR)/mipsup.c
		$(CC_CMD) $(PROCESSOR)/mipsup.c

$(OP_FILES):	$(PROCESSOR)/optab.base optab.awk
		cd $(PROCESSOR); awk -f ../optab.awk $(AWKLIST) optab.base \
				>optab.tail;
		#
		# NOTE: awk also builds a new "$(PROCESSOR)/ops.tail" file.
		#
		cat ops.head $(PROCESSOR)/ops.tail >$(PROCESSOR)/ops.h
		touch $(OP_FILES)

$(PROCESSOR)/optab.c:	optab.head $(PROCESSOR)/optab.base optab.awk $(OP_FILES)
		cat optab.head $(PROCESSOR)/optab.tail >$(PROCESSOR)/optab.c

optab.o:	$(PROCESSOR)/optab.c $(PROCESSOR)/ops.h
		$(CC_CMD) $(PROCESSOR)/optab.c

vtrace.o:	$(PROCESSOR)/vtrace.c $(PROCESSOR)/peep.c $(OPTIM_H)
		$(CC_CMD) $(PROCESSOR)/vtrace.c 

#-------------------------

install:	optim
		-rm -f $(LIBDIR)/optim
		cp optim $(LIBDIR)
		strip $(LIBDIR)/optim
		chmod $(PROT) $(LIBDIR)/optim
		chgrp $(GRP) $(LIBDIR)/optim
		chown $(OWN) $(LIBDIR)/optim

#--------------------------

save:	$(LIBDIR)/optim
	-rm -f $(LIBDIR)/optim.back
	cp $(LIBDIR)/optim $(LIBDIR)/optim.back

#--------------------------

uninstall:	$(LIBDIR)/optim.back
		-rm -f $(LIBDIR)/optim
		cp $(LIBDIR)/optim.back $(LIBDIR)/optim

#--------------------------

clean:
	-rm -f $(OFILES) $(OP_FILES)

#--------------------------

clobber shrink:	clean
		-rm -f $(PROCESSOR)/optab.c optim ops.h */ops.h

#--------------------------

lint:	$(CFILES) $(PROCESSOR)/peep.c
	$(LINT) $(LFLAGS) $(INCLIST) $(CFILES) >lint.out

#--------------------------

cxref:	$(CFILES) $(PROCESSOR)/peep.c
	cxref -c $(INCLIST) $(CFILES) -o optim.xref

#--------------------------

cflow:	$(CFILES) $(PROCESSOR)/peep.c
	cflow -r $(INCLIST) $(CFILES) >optim.cflow
