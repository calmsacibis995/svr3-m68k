#
#	MC68000 C STARTUP MAKEFILE
#
#	@(#)makefile	6.2	
#

CC=	cc
CFLAGS=	-O

OFILES=		crt0.o  initfpu.o
PFILES=		mcrt0.o initfpu.p
FILES=		$(OFILES) $(PFILES)

prof:	$(PFILES)

nonprof:	$(OFILES)

all:		$(FILES)

#-------------------------

.s.o:
	@echo $*.s:
	$(CC) $(CFLAGS) -c $*.s

#-------------------------
initfpu.p: initfpu.o
	# Since the routine "initfpu" is part of initialization code,
	# it doesn't have profiling info in it.  So just copy initfpu.o.
	cp initfpu.o initfpu.p

#-------------------------

clean:
		rm -f *.o

#--------------------------

clobber:	clean
