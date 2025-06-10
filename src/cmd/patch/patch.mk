#	SCCS:	@(#)patch.mk	2.1	

OL = 
TESTDIR = .
INSUB = /etc/install -f $(UBIN)
UBIN = $(OL)/usr/bin
CFLAGS = -O
LDFLAGS = -s
SOURCE = patch.c pf68.c
OBJECT = patch.o pf68.o
LHEADS = dd68.h

compile all: patch

patch:	$(OBJECT)
	$(CC) $(LDFLAGS) -o $(TESTDIR)/patch $(OBJECT)
	chgrp bin $(TESTDIR)/patch
	chmod 755 $(TESTDIR)/patch
	chown bin $(TESTDIR)/patch

patch.o: patch.c dd68.h
	$(CC) $(CFLAGS) -c patch.c

pf68.o: pf68.c
	$(CC) $(CFLAGS) -c pf68.c

install: all
	$(INSUB) $(TESTDIR)/patch

clean:
	rm -f $(OBJECT)

clobber:
	  rm -f $(TESTDIR)/patch
