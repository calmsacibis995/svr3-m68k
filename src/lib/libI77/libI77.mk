#	@(#)libI77.mk	7.2 

OWN=	bin		#file owner
GRP=	bin		#file group
PROT=	664		#protections

ROOT = 
INCROOT =
TESTDIR = .
LIBNAME = $(TESTDIR)/libI77.a
INSDIR = $(ROOT)/usr/lib
FRC =
CFLAGS = -O
OBJECTS =\
	Version.o\
	backspace.o\
	dfe.o\
	due.o\
	iio.o\
	inquire.o\
	rewind.o\
	rsfe.o\
	rdfmt.o\
	sue.o\
	uio.o\
	wsfe.o\
	sfe.o\
	fmt.o\
	nio.o\
	lio.o\
	lread.o\
	open.o\
	close.o\
	util.o\
	endfile.o\
	wrtfmt.o\
	err.o\
	fmtlib.o\
	ecvt.o\
	ltostr.o
FILES=$(OBJECTS:.o=.c)

all:	$(LIBNAME)

lio.o: lio.h lio.c
	$(CC) -c $(CFLAGS) $*.c
	chmod $(PROT) $*.o
	chgrp $(GRP)  $*.o
	chown $(OWN)  $*.o

lread.o: lio.h lread.c
	$(CC) -c $(CFLAGS) $*.c
	chmod $(PROT) $*.o
	chgrp $(GRP)  $*.o
	chown $(OWN)  $*.o

.c.o:
	$(CC) -c $(CFLAGS) $*.c
	chmod $(PROT) $*.o
	chgrp $(GRP)  $*.o
	chown $(OWN)  $*.o

archive: $(OBJECTS)
	ar rv $(LIBNAME) $(OBJECTS)

$(LIBNAME):	archive $(INCROOT)/usr/include/stdio.h fio.h
	if pdp11 ; then strip $(LIBNAME) ; elif m68k ; then : ; else strip -r $(LIBNAME) ; ar ts $(LIBNAME) ; fi

install:	all save
		rm -f $(INSDIR)/$(LIBNAME)
		cp $(LIBNAME) $(INSDIR)/$(LIBNAME)
		chmod $(PROT) $(INSDIR)/$(LIBNAME)
		chmod -x $(INSDIR)/$(LIBNAME)
		chgrp $(GRP) $(INSDIR)/$(LIBNAME)
		chown $(OWN) $(INSDIR)/$(LIBNAME)

#--------------------------

uninstall:	$(INSDIR)/OLDlibI77.a
		-rm -f $(INSDIR)/libI77.a
		cp $(INSDIR)/OLDlibI77.a $(INSDIR)/libI77.a
		chmod $(PROT) $(INSDIR)/libI77.a
		chmod -x $(INSDIR)/libI77.a
		chgrp $(GRP) $(INSDIR)/libI77.a
		chown $(OWN) $(INSDIR)/libI77.a

#--------------------------

save:		
		if test -f $(INSDIR)/libI77.a ; \
		then \
			rm -f $(INSDIR)/OLDlibI77.a ; \
			cp $(INSDIR)/libI77.a $(INSDIR)/OLDlibI77.a ; \
		fi

clean:
	-rm -f *.o

clobber:	clean
	-rm -f $(LIBNAME)

FRC:
