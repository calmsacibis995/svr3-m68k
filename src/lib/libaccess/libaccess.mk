#		Copyright (c) 1985 Motorola, Inc.   #
#		All Rights Reserved		#
#	@(#)makefile	1.1	10/3/85

ROOT = 
LIBNAME = $(ROOT)/usr/lib/libaccess.a
INCLD=$(ROOT)/usr/include
SINCLD =$(INCLD)
FRC = FRC


CFLAGS =  -O  -I$(INCLD) 

FILES =\
	$(LIBNAME)(ckmode.o)\
	$(LIBNAME)(getnum.o)\
	$(LIBNAME)(getperms.o)\
	$(LIBNAME)(strins.o)\
	$(FRC)

.PRECIOUS:	$(LIBNAME)

$(LIBNAME):	$(FILES)

$(INCLD)/access.h:	$(FRC)
	cd $(ROOT)/usr/src/head ;  echo access.h | cpio -pdvmu $(INCLD)

.c.a:
	$(CC) -c $(CFLAGS) $<
	chmod 664 $*.o
	chgrp bin $*.o
	chown bin $*.o
	ar rv $@ $*.o
	-rm -f $*.o

$(LIBNAME)(ckmode.o): ckmode.c\
	$(INCLD)/access.h\
	$(SINCLD)/stdio.h\
	$(FRC)
$(LIBNAME)(getnum.o): getnum.c\
	$(INCLD)/access.h\
	$(SINCLD)/stdio.h\
	$(FRC)
$(LIBNAME)(getperms.o): getperms.c\
	$(INCLD)/access.h\
	$(SINCLD)/stdio.h\
	$(FRC)
$(LIBNAME)(strins.o): strins.c\
	$(INCLD)/access.h\
	$(SINCLD)/stdio.h\
	$(FRC)

FRC:

install: $(INCLD)/access.h $(FILES)

clean:
	-rm -f *.o

clobber:	clean
