#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)ncheck:ncheck.mk	1.4"
ROOT =
TESTDIR = .
FRC =
INS = install
IFLAG = -i
LDFLAGS = -s -n
INSDIR = -n $(ROOT)/usr/bin
CFLAGS = -O
FFLAG =
SMFLAG=

all: ncheck ncheck1b ncheck16b

ncheck: ncheck.c 
	if [ x$(IFLAG) != x-i ]  ; then \
	$(CC) $(LDFLAGS) -Dsmall=-1 $(CFLAGS)  $(IFLAG) -o $(TESTDIR)/ncheck ncheck.c $(LDLIBS); \
	else $(CC) $(LDFLAGS) $(CFLAGS) $(IFLAG) -DFsTYPE=2 -o $(TESTDIR)/ncheck ncheck.c $(LDLIBS); \
	fi

ncheck1b: ncheck.c 
	$(CC) $(LDFLAGS) $(CFLAGS) $(IFLAG) -DFsTYPE=1 -o $(TESTDIR)/ncheck1b ncheck.c $(LDLIBS);

ncheck16b: ncheck.c 
	$(CC) $(LDFLAGS) $(CFLAGS) $(IFLAG) -DFsTYPE=5 -o $(TESTDIR)/ncheck16b ncheck.c $(LDLIBS);

test:
	rtest $(TESTDIR)/ncheck

install: all
	$(INS) $(INSDIR) $(TESTDIR)/ncheck $(ROOT)/etc
	$(INS) $(INSDIR) $(TESTDIR)/ncheck1b $(ROOT)/etc
	$(INS) $(INSDIR) $(TESTDIR)/ncheck16b $(ROOT)/etc

clean:

clobber: clean
	-rm -f $(TESTDIR)/ncheck $(TESTDIR)/ncheck1b $(TESTDIR)/ncheck16b

FRC:
