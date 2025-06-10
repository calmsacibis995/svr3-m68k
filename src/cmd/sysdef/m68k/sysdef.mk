#	@(#)sysdef.mk	6.1	

LDFLAGS	= -s
TESTDIR	= .
OL	=
ETC	= $(OL)/etc 
INSE	= /etc/install -f $(ETC)

compile all:		$(TESTDIR)/sysdef

$(TESTDIR)/sysdef:	sysdef.o
			$(CC) $(LDFLAGS) -o $(TESTDIR)/sysdef sysdef.o

sysdef.o:		sysdef.c
			$(CC) $(CFLAGS) -c sysdef.c

install:		all
			$(INSE) $(TESTDIR)/sysdef

clean:
			rm -f sysdef.o

clobber:		clean
			rm -f $(TESTDIR)/sysdef
