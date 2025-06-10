#
# "$Header: /grp/mat/53/utils/ixf/RCS/ixf.mk,v 1.2 87/01/27 16:11:56 mat Exp $"
#

ROOT =
TESTDIR = .
INSDIR = $(ROOT)/usr/bin
INSU = cpset

CFLAGS = -g -DDEBUG
CFLAGS =

LDFLAGS =
LDFLAGS = -s

SOURCE = ixf.c
OBJECT = ixf.o
CC=cc

all: ixf

ixf:	$(OBJECT)
	$(CC) $(LDFLAGS) -o $(TESTDIR)/ixf $(OBJECT)

ixf.o: ixf.c
	$(CC) $(CFLAGS) -c ixf.c

install: all
	$(INSU) $(TESTDIR)/ixf $(INSDIR)/ixf

clean:
	-rm -f *.o

clobber: clean
	-rm -f $(TESTDIR)/ixf
