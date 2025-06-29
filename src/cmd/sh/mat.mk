#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)sh:sh.mk	1.14"

TESTDIR = .
FRC =
ROOT= /rl
INS = install 
INSDIR = 
CFLAGS = -O -DNICE -DACCT -DNICEVAL=4
LDFLAGS = -n -s

OFILES = setbrk.o blok.o stak.o cmd.o fault.o main.o word.o string.o\
name.o args.o xec.o service.o error.o io.o print.o macro.o expand.o\
ctype.o msg.o test.o defs.o echo.o hash.o hashserv.o pwd.o func.o

all: sh

sh: $(SFILES) $(OFILES)
	$(CC) $(LDFLAGS) $(SFILES) $(OFILES) -o $(TESTDIR)/sh 

$(OFILES):	defs.h $(FRC)

ctype.o:	ctype.h
		if [ "${_ID_}" ];\
		then \
			$(CC) $(CFLAGS) -c ctype.c; \
		elif [ "${_SH_}" ]; \
		then \
			CC=$(CC) $(_SH_) ./:fix ctype; \
		else \
			CC=$(CC) sh ./:fix ctype; \
		fi

xec.o:		xec.c
	set +e; if u370;\
	then \
	$(CC) $(CFLAGS) -b1,0 -c xec.c;\
	else \
	$(CC) $(CFLAGS) -c xec.c;\
	fi

service.o:	service.c
	$(CC) $(CFLAGS) -I$(ROOT)/usr/src/cmd/acct -c service.c

blok.o fault.o main.o stak.o:	brkincr.h

io.o main.o:	dup.h

xec.o hash.o hashserv.o:	hash.h

cwd.o:	mac.h

cmd.o macro.o main.o msg.o word.o xec.o:	sym.h

main.o:	timeout.h


msg.o:		msg.c $(FRC)
		if [ "${_ID_}" ];\
		then \
			$(CC) $(CFLAGS) -c msg.c; \
		elif [ "${_SH_}" ]; \
		then \
			CC=$(CC) $(_SH_) ./:fix msg; \
		else \
			CC=$(CC) sh ./:fix msg; \
		fi

test:
	  rtest $(TESTDIR)/sh

install:  all
	  $(INS) -o -n $(ROOT)/bin $(TESTDIR)/sh $(INSDIR)
	  @if [ -f $(ROOT)/bin/rsh ]; \
	  then \
		mv $(ROOT)/bin/rsh $(ROOT)/bin/OLDrsh; \
	  fi
	  ln $(ROOT)/bin/sh $(ROOT)/bin/rsh
	  sync

clean:
	  -rm -f *.o

clobber:  clean
	  -rm -f $(TESTDIR)/sh
	  -rm -f $(ROOT)/bin/OLDrsh

FRC:
