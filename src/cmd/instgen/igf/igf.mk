#
# "$Header: /grp/mat/53/utils/igf/RCS/igf.mk,v 1.2 87/01/27 16:18:02 mat Exp $"
#

ROOT =
TESTDIR = .
INSDIR = $(ROOT)/usr/bin
INSU = cpset

CFLAGS = -g -DDEBUG
CFLAGS = -O

LDFLAGS =
LDFLAGS = -s

CC=cc

SOURCE = igf.c readtbl.c writvid.c writboot.c writdir.c writos.c \
	 writram.c writarc.c strings.c io.c
OBJECT = igf.o readtbl.o writvid.o writboot.o writdir.o writos.o \
	 writram.o writarc.o strings.o io.o

all: igf

igf:	$(OBJECT)
	$(CC) $(LDFLAGS) -o $(TESTDIR)/igf $(OBJECT) -lld

igf.o: igf.c igf.h ../com/common.h
	$(CC) $(CFLAGS) -c igf.c

readtbl.o: readtbl.c igf.h ../com/common.h
	$(CC) $(CFLAGS) -c readtbl.c

writvid.o: writvid.c igf.h ../com/common.h
	$(CC) $(CFLAGS) -DVME131 -c writvid.c

writboot.o: writboot.c igf.h ../com/common.h
	$(CC) $(CFLAGS) -c writboot.c

writdir.o: writdir.c igf.h ../com/common.h
	$(CC) $(CFLAGS) -c writdir.c

writos.o: writos.c igf.h ../com/common.h
	$(CC) $(CFLAGS) -c writos.c

writram.o: writram.c igf.h ../com/common.h
	$(CC) $(CFLAGS) -c writram.c

writarc.o: writarc.c igf.h ../com/common.h
	$(CC) $(CFLAGS) -c writarc.c

io.o: io.c igf.h ../com/common.h
	$(CC) $(CFLAGS) -c io.c

strings.o: strings.c igf.h ../com/common.h
	$(CC) $(CFLAGS) -c strings.c

install: all
	$(INSU) $(TESTDIR)/igf $(INSDIR)/igf

clean:
	-rm -f *.o

clobber: clean
	-rm -f $(TESTDIR)/igf
