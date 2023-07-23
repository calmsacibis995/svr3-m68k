#
# Makefile for revised versions of create and upgrade
#

# internal rules
.c.o:
	$(CC) $(CFLAGS) $(STATE) -DMOD_$* $<

CC = cc
CFLAGS = -c -O

#	select for debugging needs
#STATE = -DDEBUG
#	select for internal releases
#STATE = -DEXP
#	select when building official "release" version.
STATE = -DRELEASE

all:	create upgrade
	@echo "create/upgrade are now up to date!"

create:	create.o checksum.o misc.o
	$(CC) create.o checksum.o misc.o -o create

upgrade:	upgrade.o checksum.o misc.o
	$(CC) upgrade.o checksum.o misc.o -o upgrade

create.o misc.o ugrade.o:	common.h

install: all
	/etc/install -f /usr/bin create 
	/etc/install -f /usr/bin upgrade 

print:	create.c upgrade.c misc.c checksum.c common.h
	pr -w130 -f  common.h create.c upgrade.c misc.c checksum.c | lp

clean:
	rm -f *.o

clobber: clean
	rm -f ./create ./upgrade
