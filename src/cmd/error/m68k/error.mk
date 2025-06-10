#	@(#)error.mk	6.1	
#
#	error makefile for m68k
#

OL =
TESTDIR = .
CFLAGS = -O
LDFLAGS = -s

ETC = $(OL)/etc
UBIN = $(OL)/usr/bin
ULIB = $(OL)/usr/lib

INSE = /etc/install -f $(ETC)
INSUB = /etc/install -f $(UBIN
INSUL = /etc/install -f $(ULIB)


compile all:	errpt errdead errdemon errstop

errpt:	errpt.o
	$(CC) $(LDFLAGS) -o $(TESTDIR)/errpt errpt.o

errpt.o:	errpt.c
	$(CC) $(CFLAGS) -c errpt.c

errdead:	errdead.o
	$(CC) $(LDFLAGS) -o $(TESTDIR)/errdead errdead.o

errdead.o:	errdead.c
	$(CC) $(CFLAGS) -c errdead.c

errdemon:	errdemon.o
	$(CC) $(LDFLAGS) -o $(TESTDIR)/errdemon errdemon.o

errdemon.o:	errdemon.c
	$(CC) $(CFLAGS) -c errdemon.c

errstop:
	cp errstop.sh errstop

install:	all
	$(INSUB) $(TESTDIR)/errpt
	$(INSE) $(TESTDIR)/errdead
	$(INSUL) $(TESTDIR)/errdemon
	$(INSE) $(TESTDIR)/errstop

clean:
	rm -f *.o

clobber:
	rm -f $(TESTDIR)/errpt $(TESTDIR)/errdead \
		$(TESTDIR)/errdemon $(TESTDIR)/errstop
