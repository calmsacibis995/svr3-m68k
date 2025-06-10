#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-port:ml/ml.mk	6.3"

LIBNAME = ../cf/lib.ml
INCRT = ..
#INCRT = /usr/include
CFLAGS = -I$(INCRT)
FRC =

MFILES = psl.m equates.m

SFILES = trap.s mmb.s paging.s start.s cswitch.s userio.s math.s misc.s end.s

FILES = $(LIBNAME)(locore.o)

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

locore.s:	$(MFILES) $(SFILES) $(FRC)
	cat $(MFILES) $(SFILES) > locore.s

clean:
	-rm -f locore.s *.o

clobber:	clean
	-rm -f $(LIBNAME)

FRC:
