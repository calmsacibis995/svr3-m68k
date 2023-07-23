#	@(#)csoffset.mk	6.1	
LDFLAGS	= -s
TESTDIR	= .
OL	=
UBIN	= $(OL)/usr/bin
INSUB	= /etc/install -f $(UBIN)
CFLAGS = -O -T -DISP -I../../include -DCC_mcc

install:	all
		$(INSUB) $(TESTDIR)/csoffset

# all: itt cstty csoffset cstrans
all: csoffset 

# itt: itt.o csinit.o
#	$(CC) $(LDFLAGS) itt.o csinit.o -o itt

# cstty: cstty.o
#	$(CC) $(LDFLAGS) cstty.o -o cstty

csoffset: csoffset.o csinit.o csxlate.o
	$(CC) $(LDFLAGS) csoffset.o csinit.o csxlate.o -o csoffset

# cstrans: cstrans.o csinit.o csxlate.o
#	$(CC) $(LDFLAGS) cstrans.o csinit.o csxlate.o -o cstrans

clean:
			rm -f  csoffset.o csxlate.o csinit.o

clobber:		clean
			rm -f $(TESTDIR)/csoffset
