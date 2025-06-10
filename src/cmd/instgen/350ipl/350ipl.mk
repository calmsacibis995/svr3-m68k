#
# 350ipl - boot program for mvme350
# $Header: /grp/mat/53/utils/350ipl/RCS/350ipl.mk,v 1.1 87/01/27 14:59:03 mat Exp $
#
#	ipl120 is obsolete (not made).
#
CC=cc
LD=ld
AS=as
LOCDEF=-DDEBUG
LOCDEF=

all:	ipl130

ipl130:	scrt0.o 130io.o prf.o 350ipl130.o
	$(LD) -o ipl130 -e start ld130.350boot scrt0.o 130io.o prf.o 350ipl130.o -lc
	mcs -d ipl130

ipl120:	scrt0.o 120io.o prf.o 350ipl120.o
	$(LD) -o ipl120 -e start ld120.350boot scrt0.o 120io.o prf.o 350ipl120.o -lc

350ipl120.o: 350ipl.c
	$(CC) -c -DMVME120  350ipl.c
	-mv 350ipl.o 350ipl120.o

350ipl130.o: 350ipl.c
	$(CC) -c $(LOCDEF) $(CFLAGS) -DMVME130 350ipl.c
	-mv 350ipl.o 350ipl130.o

prf.o: prf.c
	$(CC) -c $(CFLAGS) prf.c

install:	all
	cpset ipl130 /stand/m68k/boots/ipl130 754 root sys

clean:
	rm -f *.o

clobber:	clean
	rm -f ipl130
