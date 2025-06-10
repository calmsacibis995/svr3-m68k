#	Makefile for track to file system conversion 

ROOT =

DIR = $(ROOT)/etc

LDFLAGS = -O -s

MAINS = fblkgen

OBJECTS =  fblkgen.o

SOURCES =  fblkgen.c

all:		$(MAINS)

fblkgen:	clobber
	$(CC) $(CFLAGS)  -o fblkgen  fblkgen.c   $(LDFLAGS)

clean:
	rm -f $(OBJECTS)

clobber:
	rm -f $(OBJECTS) $(MAINS)

install: all
	cpset $(MAINS) $(DIR)
