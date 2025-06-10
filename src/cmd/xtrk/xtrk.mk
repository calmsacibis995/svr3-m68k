#	Makefile for track to file system conversion 

ROOT =

DIR = $(ROOT)/etc

INC = $(ROOT)/usr/include

LDFLAGS = -O -s

CFLAGS = -I$(INC)

MAINS = xtrk

OBJECTS =  xtrk.o

SOURCES =  xtrk.c

all:		$(MAINS)

xtrk:	clobber
	$(CC) $(CFLAGS) $(LDFLAGS) -o xtrk  xtrk.c

clean:
	rm -f $(OBJECTS)

clobber:
	rm -f $(OBJECTS) $(MAINS)

install: all
	cpset $(MAINS) $(DIR)/fblkgen
	ln $(DIR)/fblkgen $(DIR)/xformtrk
