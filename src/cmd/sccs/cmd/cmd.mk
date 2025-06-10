#
#	@(#)cmd.mk	6.1       
#

# ***** RELEASE 2 MAKE FILE FOR RELEASE 2 FLAVOR SCCS UTILITIES.
# ***** THE RELEASE 3 VERSIONS HAVE ASSEMBLY CODE DEPENDENCIES
# ***** WHICH HAVE NOT BEEN PORTED AS YET

OL =
TESTDIR=.
UBIN =$(OL)/usr/bin
INSUB = /etc/install -f $(UBIN)
HDR = ../hdr
HELPLOC = $(OL)/usr/lib/help

LIBS = ../lib/comobj.a	\
	../lib/mpwlib.a	\
	../lib/cassi.a	\
	-lPW

CFLAGS = -O

LDFLAGS = -s

B02 = `if u370; then echo '-b0,2'; fi`
B12 = `if u370; then echo '-b1,2'; fi`

ARGS =

#	help is not used in Rel. 3.  New help(1) provided

C_CMDS = admin	\
	cdc	\
	comb	\
	delta	\
	get	\
	prs	\
	rmdel	\
	sact	\
	unget	\
	val	\
	vc	\
	what

CMDS = $(C_CMDS)	\
	sccsdiff

compile all:	$(LIBS) $(CMDS) help2

admin:	admin.o	$(LIBS)
	$(CC) $(LDFLAGS) admin.o $(LIBS) -o admin

admin.o:	admin.c
	$(CC) -c $(CFLAGS) $(B12) admin.c

cdc:	rmchg
	-ln	rmchg cdc

comb:	comb.o	$(LIBS)
	$(CC) $(LDFLAGS) comb.o $(LIBS) -o comb

comb.o:	comb.c
	$(CC) -c $(CFLAGS) $(B02) comb.c

delta:	delta.o	$(LIBS)
	$(CC) $(LDFLAGS) delta.o $(LIBS) -o delta

delta.o:	delta.c
	$(CC) -c $(CFLAGS) $(B02) delta.c

get:	get.o	$(LIBS)
	$(CC) $(LDFLAGS) get.o $(LIBS) -o get

get.o:	get.c
	$(CC) -c $(CFLAGS) $(B02) get.c

help:	help.o
	$(CC) $(LDFLAGS) help.o -o help

help.o:	help.c
	$(CC) -c $(CFLAGS) help.c

help2:	help2.o	$(LIBS)
	$(CC) $(LDFLAGS) help2.o $(LIBS) -o help2

help2.o: help2.c
	$(CC) -c $(CFLAGS) help2.c

prs:	prs.o	$(LIBS)
	$(CC) $(LDFLAGS) prs.o $(LIBS) -o prs

prs.o:	prs.c
	$(CC) -c $(CFLAGS) $(B12) prs.c
	
rmdel:	rmchg $(LIBS)
	-ln rmchg rmdel

rmchg:	rmchg.o $(LIBS)
	$(CC) $(LDFLAGS) rmchg.o $(LIBS) -o rmchg

rmchg.o:	rmchg.c
	$(CC) -c $(CFLAGS) $(B02) rmchg.c

sact:	unget
	-ln unget sact

sccsdiff:	sccsdiff.sh
	-cp sccsdiff.sh sccsdiff

unget:	unget.o	$(LIBS)
	$(CC) $(LDFLAGS) unget.o $(LIBS) -o unget

unget.o:	unget.c
	$(CC) -c $(CFLAGS) $(B02) unget.c

val:	val.o	$(LIBS)
	$(CC) $(LDFLAGS) val.o $(LIBS) -o val

val.o:	val.c
	$(CC) -c $(CFLAGS) $(B02) val.c

vc:	vc.o	$(LIBS)
	$(CC) $(LDFLAGS) vc.o $(LIBS) -o vc

vc.o:	vc.c
	$(CC) -c $(CFLAGS) vc.c

what:	what.o	$(LIBS)
	$(CC) $(LDFLAGS) what.o $(LIBS) -o what

what.o:	what.c
	$(CC) -c $(CFLAGS) what.c

$(LIBS):
	cd ../lib; $(MAKE) -f lib.mk

install:	all
	$(INSUB) $(TESTDIR)/admin
	$(INSUB) $(TESTDIR)/cdc
	$(INSUB) $(TESTDIR)/comb
	$(INSUB) $(TESTDIR)/delta
	$(INSUB) $(TESTDIR)/get
	$(INSUB) $(TESTDIR)/help
	$(INSUB) $(TESTDIR)/prs
	$(INSUB) $(TESTDIR)/rmdel
	$(INSUB) $(TESTDIR)/sact
	$(INSUB) $(TESTDIR)/unget
	$(INSUB) $(TESTDIR)/val
	$(INSUB) $(TESTDIR)/vc
	$(INSUB) $(TESTDIR)/what
	$(INSUB) $(TESTDIR)/sccsdiff
	-chmod 775 $(UBIN)/$(CMDS) 
	-chgrp bin $(UBIN)/$(CMDS) 
	-chown bin $(UBIN)/$(CMDS) 
	if [ ! -d $(HELPLOC) ] ; then mkdir $(HELPLOC) ; fi
	if [ ! -d $(HELPLOC)/lib ] ; then mkdir $(HELPLOC)/lib ; fi
	-mv help2 $(HELPLOC)/lib
	-chmod 775 $(HELPLOC)/lib/help2
	-chgrp bin $(HELPLOC)/lib/help2
	-chown bin $(HELPLOC)/lib/help2

clean:
	-rm -f *.o
	-rm -f rmchg

clobber:	clean
	-rm -f $(TESTDIR)/$(CMDS) $(TESTDIR)/help2
