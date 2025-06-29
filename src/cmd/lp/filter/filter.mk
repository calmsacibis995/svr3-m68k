#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)lp:filter/filter.mk	1.7"
#	filter make file

ROOT =
OL	= $(ROOT)/
SL	= $(ROOT)/usr/src/cmd
RDIR	= $(SL)/lp/filter
INS	= :
REL	= current
OWN	= bin
GRP	= bin
LIST	= lp
BIN	= $(OL)usr/lib
IFLAG	= -n
CFLAGS	= -O
LDFLAGS	= -s $(IFLAG)
#SOURCE	= hp2631a.c prx.c pprx.c 5310.c
SOURCE	= hp2631a.c prx.c pprx.c 
#FILES	= hp2631a.o prx.o pprx.o 5310.o
FILES	= hp2631a.o prx.o pprx.o 
MAKE	= make
INSBIN	= $(INS) cp $@ $(BIN); \
	  $(INS) chmod 775 $(BIN)/$@; \
	  $(INS) if [ "$(OL)" = "/" ]; \
	     $(INS) then cd $(BIN); $(INS) chgrp $(GRP) $@; $(INS) chown $(OWN) $@; \
	  $(INS) fi

#compile all: hp2631a prx pprx 
compile all: hp2631a prx pprx 5310
	:

5310:	5310.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o 5310 5310.o
	$(INSBIN)
hp2631a:	hp2631a.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o hp2631a hp2631a.o
	$(INSBIN)
prx:	prx.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o prx prx.o
	$(INSBIN)
pprx:	pprx.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o pprx pprx.o
	$(INSBIN)

install:
	$(MAKE) -f filter.mk INS= OL=$(OL) $(ARGS)
#######################################################################
#################################DSL only section - for development use

build:	bldmk
	get -p -r`gsid filter $(REL)` s.filter.src $(REWIRE) | ntar -d $(RDIR) -g
bldmk:  ;  get -p -r`gsid filter.mk $(REL)` s.filter.mk > $(RDIR)/filter.mk

listing:
	pr filter.mk $(SOURCE) | $(LIST)
listmk: ;  pr filter.mk | $(LIST)

edit:
	get -e -p s.filter.src | ntar -g

delta:
	ntar -p $(SOURCE) > filter.src
	delta s.filter.src
	rm -f $(SOURCE)

mkedit:  ;  get -e s.filter.mk
mkdelta: ;  delta s.filter.mk
#######################################################################

clean:
	rm -f $(FILES)

clobber:  clean
	rm -f hp2631a prx pprx 5310

delete:	clobber
	rm -f $(SOURCE)
