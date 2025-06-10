#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-port:stub/stub.mk	6.3"

LIBNAME = ../cf/lib.stub
INCRT = ..
#INCRT = /usr/include
CFLAGS = -I$(INCRT)
FRC =

MFILES = ../ml/psl.m ../ml/equates.m

SFILES = stream.s du.s

FILES = $(LIBNAME)(stub.o)

all:	pickup $(LIBNAME) 

.PRECIOUS:	$(LIBNAME)

pickup:
	-@if [ "`ls *.o 2>/dev/null`" ] ; \
	then \
		echo "\tar ruv $(LIBNAME) *.o" ; \
		ar ruv $(LIBNAME) *.o ; \
		echo "\trm -f *.o" ; \
		rm -f *.o  ; \
	fi

$(LIBNAME):	$(FILES)
	ar ruv $(LIBNAME) *.o
	rm -f *.o

.s.a:
	$(CC) $(CFLAGS) -c $<

stub.s:	$(MFILES) $(SFILES) $(FRC)
	cat $(MFILES) $(SFILES) > stub.s

clean:
	-rm -f stub.s *.o

clobber:	clean
	-rm -f $(LIBNAME)

FRC:
