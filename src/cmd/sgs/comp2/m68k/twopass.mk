#	@(#)twopass.mk	7.1	
#	MC68000 PCC2 TWOPASS MAKEFILE
#
#	@(#) makefile:	2.3 83/07/08
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
DEFLIST=	$(FLEX)
TWOPASS=	-DTWOPASS -DITREES
LDFLAGS=
YFLAGS=

CC_CMD=	$(CC) -c $(CFLAGS) $(INCLIST) $(DEFLIST) $(ODEFLIST) $(TWOPASS)
YACC_CMD=	$(YACC) $(YFLAGS)

# use M as a short-hand for the awkward PCC2COM
# (M for Machine independent part)

M=	$(PCC2COM)


# OFILES= cgram.o xdefs.o scan.o pftn.o trees.o optim.o local.o reader.o\
	local2.o debug.o match.o allo.o comm1.o table.o cost.o cgen.o
# CFILES=	$M/cgram.c $M/xdefs.c $M/scan.c $M/pftn.c $M/cgen.c $M/cost.c \
	$M/trees.c $M/optim.c local.c $M/reader.c \
	local2.c debug.c $M/match.c $M/allo.c $M/comm1.c \
	table.c

C0_OFILES= trees.o scan.o cgram.o xdefs.o pftn.o optim.o local.o comm1.o \
		debug.o xdefs.o

C0_CFILES= $M/trees.c $M/scan.c $M/cgram.c $M/xdefs.c $M/pftn.c $M/optim.c\
	$M/comm1.c $M/xdefs.c debug.c local.c

C1_OFILES= reader.o local2.o match.o allo.o comm2.o table.o\
	cgen.o cost.o xdefs.o

C1_CFILES= $M/reader.c $M/match.c $M/allo.c $M/comm2.c $M/cgen.c\
	$M/cost.c $M/xdefs.c local2.c table.c

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
		/bin/cc $(CFLAGS) $(LDFLAGS) -I. -I$M -o sty y.tab.c 
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

table.o:	$M/manifest.h $M/mfile2.h macdefs.h table.c
		$(CC_CMD) table.c

table.c:	sty stin
		-@echo "Expect 2 \"... may be covered by ...\" messages"
		./sty <stin >table.c

cost.o:		$M/manifest.h $M/mfile2.h macdefs.h $M/cost.c
		$(CC_CMD) $M/cost.c

cgen.o:		$M/manifest.h $M/mfile2.h macdefs.h $M/cgen.c
		$(CC_CMD) $M/cgen.c

reader.o:	$M/manifest.h $M/mfile2.h macdefs.h $M/reader.c
		$(CC_CMD) $M/reader.c

local2.o:	$M/manifest.h $M/mfile2.h macdefs.h
		$(CC_CMD) local2.c

debug.o:	$M/mfile1.h macdefs.h $ROOT/usr/include/storclass.h
		$(CC_CMD) debug.c

match.o:	$M/manifest.h $M/mfile2.h macdefs.h $M/match.c $(SDBINC)/sdb.h
		$(CC_CMD) $M/match.c

allo.o:		$M/manifest.h $M/mfile2.h macdefs.h $M/allo.c
		$(CC_CMD) $M/allo.c

#-------------------------

install:	ccom
		-rm -f $(LIBDIR)/ccom
		cp ccom $(LIBDIR)
		strip $(LIBDIR)/ccom

#--------------------------

save:	$(LIBDIR)/ccom
	-rm -f $(LIBDIR)/ccom.back
	cp $(LIBDIR)/ccom $(LIBDIR)/ccom.back

#--------------------------

uninstall:	$(LIBDIR)/ccom.back
		-rm -f $(LIBDIR)/ccom
		cp $(LIBDIR)/ccom.back $(LIBDIR)/ccom

#--------------------------

clean:
	-rm -f $(OFILES)

#--------------------------

clobber:	clean
		-rm -f ccom table.c sty $M/cgram.c

#--------------------------

lint:	$(CFILES)
	$(LINT) $(LFLAGS) -I. -I$M -I$(SDBINC) $(CFILES) >lint.out
