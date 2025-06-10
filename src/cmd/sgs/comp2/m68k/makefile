#	@(#)makefile	7.4		86/09/06
#
#	MC68000 PCC2 TWOPASS MAKEFILE
#		PROCESSOR must be set to M68000, M68010, or M68020
#
#
ROOT=	

BINDIR=	$(ROOT)/bin
LIBDIR=	$(ROOT)/lib

PCC2COM=	../common
# Directory for sdb.h
SDBINC=	.

FLEX=	-DFLEXNAMES

CC=	cc
LINT=	lint
YACC=	yacc
LFLAGS=	-p
CFLAGS=	-O
# CFLAGS=	-g
INCLIST=	-I. -I $(PCC2COM) -I$(SDBINC)
#
#	conditional compilation variables are declared 
#	in the DEFLIST and are passed to cpp.
#
#	variables:
#		STKCK	- causes inclusion of code to generate
#			  stack fault preclusion code at function
#			  entry.  targeted for MC68000.
#		PROFILE - causes inclusion of profiling code -
#			  conditional compilation can be removed if desired.
#
#		TWOPASS - causes inclusion of code when defined
#			  to generate two pass compiler.  this "#undef"s
#			  ONEPASS in macdefs.h.  if not defined, a one
#			  pass compiler is created.
#
#		ITREES  - Use icon based passing to pass all information
#			  to the back end.
#

ODEFLIST=	-DSTKCK -DPROFILE
DEFLIST=	$(FLEX) -Dm68k
TWOPASS=	-DTWOPASS -DITREES
LDFLAGS=
YFLAGS=
#
# IMPORTANT NOTE FOR 68020 USERS: If you ever intend on setting the FPU
# environment variable to M68881, then you had better generate the compiler
# with FPU=M68881 set below. The conservative approach would be to always
# define FPU to M68881 when generating a 68020 compiler.
#
#FPU=NOFPU	# for 68000 and 68010 compilers (default case)
FPU=M68881	# for 68020/881 support (set the FPU environ variable to get it)

CC_CMD=	$(CC) -c $(CFLAGS) $(INCLIST) $(DEFLIST) $(ODEFLIST) $(TWOPASS) -D$(PROCESSOR) -D$(FPU)
YACC_CMD=	$(YACC) $(YFLAGS)

# use M as a short-hand for the awkward PCC2COM
# (M for Machine independent part)

M=	$(PCC2COM)


# OFILES= cgram.o xdefs.o scan.o pftn.o trees.o optim.o local.o reader.o\
	local2.o debug.o match.o allo.o comm1.o tablea.o tableb.o cost.o cgen.o
# CFILES=	$M/cgram.c $M/xdefs.c $M/scan.c $M/pftn.c $M/cgen.c $M/cost.c \
	$M/trees.c $M/optim.c local.c $M/reader.c \
	$(PROCESSOR)/local2.c debug.c $M/match.c $M/allo.c $M/comm1.c \
	$(PROCESSOR)/tablea.c $(PROCESSOR)/tableb.c

C0_OFILES= trees.o scan.o cgram.o xdefs.o pftn.o optim.o local.o comm1.o \
		debug.o xdefs.o

C0_CFILES= $M/trees.c $M/scan.c $M/cgram.c $M/xdefs.c $M/pftn.c $M/optim.c\
	$M/comm1.c $M/xdefs.c debug.c local.c

C1_OFILES= reader.o local2.o match.o allo.o comm2.o tablea.o tableb.o\
	cgen.o cost.o xdefs.o

C1_CFILES= $M/reader.c $M/match.c $M/allo.c $M/comm2.c $M/cgen.c\
	$M/cost.c $M/xdefs.c $(PROCESSOR)/local2.c $(PROCESSOR)/tablea.c\
	$(PROCESSOR)/tableb.c

build:	c0 c1
#-------------------------

c0:	$(C0_OFILES)
		$(CC) $(CFLAGS) $(LDFLAGS) -o c0 $(C0_OFILES)

c1:	$(C1_OFILES)
		$(CC) $(CFLAGS) $(LDFLAGS) -o c1 $(C1_OFILES)

sty:		$M/manifest.h $M/sty.y macdefs.h $M/mfile2.h $M/common
		-@echo "Expect 3 reduce/reduce conflicts"
		$(YACC_CMD) $M/sty.y
	#
	# always use the resident compiler to generate sty.
	# whether generating a cross-compiler or a resident
	# on a non-target machine.
	#
		/bin/cc $(CFLAGS) $(LDFLAGS) -I. -I$M -o sty y.tab.c $M/shptbl.c
		rm y.tab.c

trees.o:	$M/manifest.h macdefs.h $M/mfile1.h $(SDBINC)/sdb.h $M/trees.c
		$(CC_CMD) $M/trees.c

optim.o:	$M/manifest.h macdefs.h $M/mfile1.h $(SDBINC)/sdb.h $M/optim.c
		$(CC_CMD) $M/optim.c

pftn.o:		$M/manifest.h macdefs.h $M/mfile1.h $(SDBINC)/sdb.h $M/pftn.c
		$(CC_CMD) $M/pftn.c

local.o:	$M/manifest.h macdefs.h $M/mfile1.h $(SDBINC)/sdb.h
		$(CC_CMD) local.c

scan.o:		$M/manifest.h macdefs.h $M/mfile1.h $(SDBINC)/sdb.h $M/scan.c
		$(CC_CMD) $M/scan.c

xdefs.o:	$M/manifest.h $M/mfile1.h $(SDBINC)/sdb.h macdefs.h $M/xdefs.c
		$(CC_CMD) $M/xdefs.c

cgram.o:	$M/manifest.h $M/mfile1.h $(SDBINC)/sdb.h macdefs.h $M/cgram.c
		$(CC_CMD) $M/cgram.c

$M/cgram.c:	$M/cgram.y
		-@echo "Expect 6 shift/reduce conflicts"
		$(YACC_CMD) $M/cgram.y
		mv y.tab.c $M/cgram.c

comm1.o:	$M/manifest.h $M/mfile1.h $(SDBINC)/sdb.h $M/common macdefs.h \
			$M/comm1.c
		$(CC_CMD) $M/comm1.c

comm2.o:	$M/manifest.h $M/mfile2.h $(SDBINC)/sdb.h $M/common macdefs.h \
			$M/comm2.c
		$(CC_CMD) $M/comm2.c

tablea.o:	$M/manifest.h $M/mfile2.h macdefs.h $(PROCESSOR)/tablea.c
		$(CC_CMD) $(PROCESSOR)/tablea.c

tableb.o:	tablea.o
		$(CC_CMD) $(PROCESSOR)/tableb.c

$(PROCESSOR)/tablea.c $(PROCESSOR)/tableb.c:	sty $(PROCESSOR)/stin
		-@echo "Expect several \"... may be covered by ...\" messages"
		./sty <$(PROCESSOR)/stin
		mv tablea.c $(PROCESSOR)/tablea.c
		mv tableb.c $(PROCESSOR)/tableb.c

cost.o:		$M/manifest.h $M/mfile2.h macdefs.h $M/cost.c
		$(CC_CMD) $M/cost.c

cgen.o:		$M/manifest.h $M/mfile2.h macdefs.h $M/cgen.c
		$(CC_CMD) $M/cgen.c

reader.o:	$M/manifest.h $M/mfile2.h macdefs.h $M/reader.c
		$(CC_CMD) $M/reader.c

local2.o:	$M/manifest.h $M/mfile2.h macdefs.h $(PROCESSOR)/local2.c
		$(CC_CMD) $(PROCESSOR)/local2.c

debug.o:	$M/mfile1.h macdefs.h $(ROOT)/usr/include/storclass.h
		$(CC_CMD) debug.c

match.o:	$M/manifest.h $M/mfile2.h macdefs.h $M/match.c $(SDBINC)/sdb.h
		$(CC_CMD) $M/match.c

allo.o:		$M/manifest.h $M/mfile2.h macdefs.h $M/allo.c
		$(CC_CMD) $M/allo.c

#-------------------------

install:	c0 c1
		-rm -f $(LIBDIR)/c0 $(LIBDIR)/c1
		cp c0 c1 $(LIBDIR)
		strip $(LIBDIR)/c0 $(LIBDIR)/c1

#--------------------------

save:	$(LIBDIR)/c0 $(LIBDIR)/c1
	-rm -f $(LIBDIR)/c0.back $(LIBDIR)/c1.back
	cp $(LIBDIR)/c0 $(LIBDIR)/c0.back
	cp $(LIBDIR)/c1 $(LIBDIR)/c1.back

#--------------------------

uninstall:	$(LIBDIR)/c0.back $(LIBDIR)/c1.back
		-rm -f $(LIBDIR)/c0 $(LIBDIR)/c1
		cp $(LIBDIR)/c0.back $(LIBDIR)/c0
		cp $(LIBDIR)/c1.back $(LIBDIR)/c1

#--------------------------

clean:
	-rm -f $(C0_OFILES) $(C1_OFILES)

#--------------------------

clobber shrink:	clean
		-rm -f c0 c1 $(PROCESSOR)/table*.c sty shptbl.o y.tab.o $M/cgram.c

#--------------------------

lint:	$(C0_CFILES) $(C1_CFILES)
	$(LINT) $(LFLAGS) -I. -I$M -I$(SDBINC) $(C0_CFILES) $(C1_CFILES) >lint.out

#--------------------------
DEFS=$(DEFLIST) $(ODEFLIST) $(TWOPASS) -D$(PROCESSOR) -D$(FPU)

cxref0: $(C0_CFILES)
	cxref -c -I. -I$M -I$(SDBINC) $(DEFS) $(C0_CFILES) -o c0.xref

cxref1: $(C1_CFILES)
	cxref -c -I. -I$M -I$(SDBINC) $(DEFS) $(C1_CFILES) -o c1.xref

cxref: cxref0 cxref1

#--------------------------
C0_CFILES2= $M/trees.c $M/scan.c $M/cgram.y $M/xdefs.c $M/pftn.c $M/optim.c\
	$M/comm1.c $M/xdefs.c debug.c local.c

cflow0:	$(C0_CFILES2)
	cflow -r -I. -I$M -I$(SDBINC) $(DEFS) $(C0_CFILES) >c0.cflow

cflow1:	$(C1_CFILES)
	cflow -r -I. -I$M -I$(SDBINC) $(DEFS) $(C1_CFILES) >c1.cflow

cflow: cflow0 cflow1

