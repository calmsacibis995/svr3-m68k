#ident	"@(#)netaddr:netaddr.mk	1.0"

ROOT=
INCRT = $(ROOT)/usr/include
INSDIR = $(ROOT)/usr/lbin
CFLAGS =
LDFLAGS = -s
INS=cpset

all:	netaddr

netaddr: netaddr.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o netaddr netaddr.o $(LDLIBS)

netaddr.o: netaddr.c
	$(CC) $(CFLAGS) -I$(INCRT) -c netaddr.c

install: all
	$(INS) netaddr $(INSDIR) 04755 root sys

clean:
	-rm -f netaddr.o

clobber: clean
	rm -f netaddr
