#	%W%
#	bootcpio - bootable table cpio

OL =
TESTDIR = .
ETC = $(OL)/etc
INSU = /etc/install -f $(ETC)
CFLAGS = -O -DDEBUG
CFLAGS = -O
LDFLAGS = -s # -i

compile all: bootcpio

$(TESTDIR)/bootcpio:	bootcpio.o
		$(CC) $(LDFLAGS) -o $(TESTDIR)/bootcpio bootcpio.o 

bootcpio.o:		bootcpio.c
		$(CC) $(CFLAGS)  -c bootcpio.c

install: all
	$(INSU) $(TESTDIR)/bootcpio

clean:
	-rm -f bootcpio.o

clobber: clean
	-rm -f $(TESTDIR)/bootcpio
