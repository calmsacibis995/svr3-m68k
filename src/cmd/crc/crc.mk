#
#ident "crc.mk -- Makefile for 'crc'"
#

# internal rules
.c.o:
	$(CC) $(CFLAGS) $(STATE) -DMOD_$* $<

# macros
CC = cc
CFLAGS = -c -O

#	select for debugging
#STATE = -DDEBUG
#	select for internal releases
#STATE = -DEXP
#	select when building official "release" version.
STATE = -DRELEASE

all:	crc
	@echo crc is now up to date!

crc:	crc.o
	$(CC) -o ./crc crc.o

print:	crc.c
	list -dlw crc.c

clean:
	rm -f crc.o

install: all
	/etc/install -f /usr/bin ./crc 

clobber:
	rm -f ./crc.o ./crc
