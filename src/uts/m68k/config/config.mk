# @(#)config.mk	6.1

INSDIR = ../cf
LDFLAGS= -s
DASHG = 
DASHO = -O
CFLAGS= $(DASHG) $(DASHO)
FRC =

all:	$(INSDIR)/config

$(INSDIR)/config: m68k/config.c
	-rm -f $(INSDIR)/config
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(INSDIR)/config m68k/config.c
	chmod -w $(INSDIR)/config

clean:
	rm -f *.o

clobber: clean
	rm -f $(INSDIR)/config
