#	@(#)dinit.mk	6.1 
#	dinit make file
OL=
TESTDIR=.
CFLAGS= -O
LDFLAGS = -s
ETC=$(OL)/etc
INSE=/etc/install -f $(ETC)

compile all:	dinit

dinit:	dinit.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o dinit dinit.c -lld

install:	all
	$(INSE) $(TESTDIR)/dinit
	chmod 755 $(ETC)/dinit
	chown root $(ETC)/dinit
	chgrp sys $(ETC)/dinit

clean:
	rm -f *.o

clobber: clean
	rm -f $(TESTDIR)/dinit
