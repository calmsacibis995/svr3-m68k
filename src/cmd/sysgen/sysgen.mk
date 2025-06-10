#	sysgen.mk		%W%
#
#	Copyright 1986, Motorola, Inc.
#	All rights reserved.
#
DEFINE=-DREL5DOT3 -DCURSES5DOT3
CFLAGS=$(DEFINE) -O
LDFLAGS=-s

HEADERS=sysgen.h sgform.h sgkey.h

SOURCES=sgconfig.c sgdesc.c sgdevice.c sgdriver.c sgfile.c \
	sgform.c sgmiscdesc.c sgyacc.y sysgen.c

OBJECTS=sgconfig.o sgdesc.o sgdevice.o sgdriver.o sgfile.o \
	sgform.o sgmiscdesc.o sgyacc.o sysgen.o

INSDIR=/usr/src/uts/m68k/sysgen
SHINSDIR=/etc

INS=install -f
CP=cp
CHMOD=chmod

.y.h:
	$(YACC) $(YFLAGS) -d $*.y
	rm -f y.tab.c
	mv y.tab.h $*.h

sysgen: $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -ly -lcurses -o sysgen

all:	install clobber

install: sysgen
	$(INS) $(INSDIR) -m 0500 -u root -g sys sysgen
	$(CP) sysgen.sh $(SHINSDIR)/sysgen
	$(CHMOD) 0500 $(SHINSDIR)/sysgen

clean:
	rm -f $(OBJECTS)

clobber: clean
	rm -f sgyacc.h y.tab.[ch] sysgen


sgyacc.h: sgyacc.y

sgyacc.o:	sysgen.h
sgconfig.o:	sysgen.h sgyacc.h
sgdesc.o:	sysgen.h sgyacc.h sgform.h
sgdriver.o:	sysgen.h sgyacc.h sgform.h sgkey.h
sgdevice.o:	sysgen.h sgyacc.h sgform.h sgkey.h
sgmiscdesc.o:	sysgen.h sgyacc.h sgform.h sgkey.h
sgform.o:	sysgen.h sgyacc.h sgform.h
sysgen.o:	sysgen.h sgyacc.h sgform.h sgkey.h
