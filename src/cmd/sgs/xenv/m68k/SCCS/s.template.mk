h04855
s 00122/00122/00116
d D 1.2 86/11/19 11:12:10 fnf 2 1
c Add warning and update for m68k.
e
s 00238/00000/00000
d D 1.1 86/07/29 14:43:56 fnf 1 0
c Baseline code from 5.3 release for 3b2
e
u
U
t
T
I 1
#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

D 2
#ident	"@(#)xenv:m32/template.mk	1.35"
E 2
I 2
#ident	"@(#)xenv:m68k/template.mk	1.35"
E 2
#
D 2
#	WE-32000 GLOBAL MAKEFILE
E 2
I 2
#			      W A R N I N G
E 2
#
I 2
#	Do not directly edit this file unless you are editing the
#	master copy under <somewhere>/src/cmd/sgs/xenv/<mach>/template.mk!
#       Otherwise your changes will be lost when the next pathedit is run.
E 2
#
I 2
#
#	MOTOROLA-M68K GLOBAL MAKEFILE
#
#
E 2
#	SGS indicates the prefix to be applied to the installed commands.
D 2
SGS=m32
E 2
I 2
SGS=
E 2
#
#	ARCH indicates the architecture of the host machine
D 2
#		AR16WR=11/70, AR32WR=vax, AR32W=ibm, 3b20, etc
E 2
I 2
#		AR16WR=11/70, AR32WR=vax, AR32W=ibm, 3b20, m68k etc
E 2
ARCH=AR32W
#
#	The following macros define the various installation directories.
#	Note that if relative pathnames are used, they must allow for the
#	"cd" issued prior to each make.
#
#	BASE is the directory under which all the sgs source lives
BASE= ../..
#
#	BINDIR is the directory for installing executable ("bin") files.
#	It should correspond to the same directory as specified in AS and LD
#	in paths.h.
D 2
BINDIR=
E 2
I 2
BINDIR=/bin
E 2
#
#	LIBDIR is the directory for installing libraries and executable
D 2
#	files not intended for direct user execution (e.g., assembler pass1).
E 2
I 2
#	files not intended for direct user execution (e.g., compiler pass1).
E 2
#	It should correspond to the same directory as specified in COMP,
#	OPTIM, AS1, and AS2 in paths.h.
D 2
LIBDIR=
E 2
I 2
LIBDIR=/lib
E 2
#
#	INCDIR is the directory for installing include files used in
#	developing the SGS and which are needed by certain users of the
#	SGS (e.g., operating system developers).  It should correspond to
#	the same directory as specified in INCDIR in paths.h.
D 2
INCDIR=
E 2
I 2
INCDIR=/tmp
E 2
#
#	Specify the byte order for this SGS instance.
D 2
#		FBO = Forward Byte Order (3B20, IBM)
E 2
I 2
#		FBO = Forward Byte Order (3B20, IBM, M68K)
E 2
#		RBO = Reverse Byte Order (DEC)
DBO=FBO
#
#	If host machine is running a pre-5.0 release of UNIX
#	then set Archive Format to "OLDAR".
#	Starting with the 5.0 UNIX (SVR1)  release the Archive Format
#	should be set to "PORTAR".
#	If building a 5.0 release on a host that is running
#	System V Rel. 2.0 (SVR2), then set ARFORMAT to PORT5AR.
#
ARFORMAT=PORTAR
#
#	Starting with the SVR2 release of UNIX,
#	if flexnames feature is desired, then set FLEX
#	to -DFLEXNAMES.
#	If host machine is running a pre-SVR2 release of UNIX
#	then set FLEX to null (ie. FLEX= ).
#
FLEX=-DFLEXNAMES
#
#	MAC parameter specifies the set of software workarounds
#	to be produced by the new sgs.
#	MAC=ABWORMAC produces sgs for WE32001 and WE32100 chips.
#	MAC=BMAC produces sgs for WE32100  o n l y.
I 2
#	MAC=<nothing> for m68k.
E 2
#
MAC=
#	This is the machine ID field. The valid values are
D 2
#	u3b15, u3b5 or u3b2.
MACH=
E 2
I 2
#	u3b15, u3b5, u3b2 or m68k.
MACH=m68k
E 2
#
#	The following parameter specifies the default include directory
#	for cpp. If not defined the value will be ROOT/usr/include.
#
D 2
USRINC=
E 2
I 2
USRINC=/usr/include
E 2
NOUSRINC=NOUSRINC
#
#
OWN=bin
GRP=bin
#
#
MAKE=make
CC=cc
FFLAG=
ENV=
ROOT=
VERSION=
LDLIBS=
D 2
LIBLD=$(BASE)/libld/m32/libld.a
E 2
I 2
LIBLD=$(BASE)/libld/m68k/libld.a
E 2
#
#
ENVPARMS=MAKE="$(MAKE)" SGS="$(SGS)" ARCH="$(ARCH)" OWN="$(OWN)" GRP="$(GRP)" \
	DBO="$(DBO)" ARFORMAT="$(ARFORMAT)" FLEX="$(FLEX)" LDLIBS="$(LDLIBS)"

CPPARMS=CC="$(CC)" FFLAG="$(FFLAG)" ENV="$(ENV)" ROOT="$(ROOT)" \
	VERSION="$(VERSION)" USRINC="$(USRINC)"

CCPARMS=CC="$(CC)" FFLAG="$(FFLAG)" ENV="$(ENV)" ROOT="$(ROOT)" \
	VERSION="$(VERSION)" USRINC="$(NOUSRINC)"

DIRPARMS=BINDIR="$(BINDIR)" LIBDIR="$(LIBDIR)"
#
#
all:	libs
D 2
	cd $(BASE)/ar/m32; $(MAKE) $(ENVPARMS) $(CCPARMS) 
	cd $(BASE)/comp2/m32; $(MAKE) $(ENVPARMS) $(CCPARMS) 
	cd $(BASE)/cpp/m32; $(MAKE) $(ENVPARMS) $(CPPARMS) PD_SYS=D_unix \
E 2
I 2
	cd $(BASE)/ar/m68k; $(MAKE) $(ENVPARMS) $(CCPARMS) 
	cd $(BASE)/comp2/m68k; $(MAKE) $(ENVPARMS) $(CCPARMS) 
	cd $(BASE)/cpp/m68k; $(MAKE) $(ENVPARMS) $(CPPARMS) PD_SYS=D_unix \
E 2
		PD_MACH=D_newmach NEW_MACH="-DPD_MY_MACH=\\\"$(MACH)\\\""
D 2
	cd $(BASE)/cmd/m32; $(MAKE) $(ENVPARMS) $(CCPARMS)
	cd $(BASE)/optim/m32; $(MAKE) $(ENVPARMS) $(CCPARMS)
	cd $(BASE)/as/m32; $(MAKE) $(ENVPARMS) $(CCPARMS) MAC=$(MAC)
	cd $(BASE)/ld/m32; $(MAKE) $(ENVPARMS) $(CCPARMS)
	cd $(BASE)/compress/m32; $(MAKE) $(ENVPARMS) $(CCPARMS)
	cd $(BASE)/convert/m32; $(MAKE) $(ENVPARMS) $(CCPARMS)
	cd $(BASE)/dis/m32; $(MAKE) $(ENVPARMS) $(CCPARMS)
	cd $(BASE)/dump/m32; $(MAKE) $(ENVPARMS) $(CCPARMS)
	cd $(BASE)/list/m32; $(MAKE) $(ENVPARMS) $(CCPARMS)
	cd $(BASE)/lorder/m32; $(MAKE) $(ENVPARMS)
	cd $(BASE)/mkshlib/m32; $(MAKE) $(ENVPARMS)
	cd $(BASE)/nm/m32; $(MAKE) $(ENVPARMS) $(CCPARMS)
	cd $(BASE)/size/m32; $(MAKE) $(ENVPARMS) $(CCPARMS)
	cd $(BASE)/strip/m32; $(MAKE) $(ENVPARMS) $(CCPARMS)
	cd $(BASE)/unix_conv/m32; $(MAKE) $(ENVPARMS) $(CCPARMS)
	cd $(BASE)/../make; /bin/make -f make.mk
E 2
I 2
	cd $(BASE)/cmd/m68k; $(MAKE) $(ENVPARMS) $(CCPARMS)
	cd $(BASE)/optim/m68k; $(MAKE) $(ENVPARMS) $(CCPARMS)
	cd $(BASE)/hloptim/m68k; $(MAKE) $(ENVPARMS) $(CCPARMS)
	cd $(BASE)/as/m68k; $(MAKE) $(ENVPARMS) $(CCPARMS) MAC=$(MAC)
	cd $(BASE)/ld/m68k; $(MAKE) $(ENVPARMS) $(CCPARMS)
	cd $(BASE)/compress/m68k; $(MAKE) $(ENVPARMS) $(CCPARMS)
	cd $(BASE)/convert/m68k; $(MAKE) $(ENVPARMS) $(CCPARMS)
	cd $(BASE)/dis/m68k; $(MAKE) $(ENVPARMS) $(CCPARMS)
	cd $(BASE)/dump/m68k; $(MAKE) $(ENVPARMS) $(CCPARMS)
	cd $(BASE)/list/m68k; $(MAKE) $(ENVPARMS) $(CCPARMS)
	cd $(BASE)/lorder/m68k; $(MAKE) $(ENVPARMS)
	cd $(BASE)/mkshlib/m68k; $(MAKE) $(ENVPARMS)
	cd $(BASE)/nm/m68k; $(MAKE) $(ENVPARMS) $(CCPARMS)
	cd $(BASE)/size/m68k; $(MAKE) $(ENVPARMS) $(CCPARMS)
	cd $(BASE)/strip/m68k; $(MAKE) $(ENVPARMS) $(CCPARMS)
	cd $(BASE)/unix_conv/m68k; $(MAKE) $(ENVPARMS) $(CCPARMS)
E 2
#
libs:
D 2
	cd $(BASE)/libld/m32; $(MAKE) $(ENVPARMS) $(CCPARMS)
E 2
I 2
	cd $(BASE)/libld/m68k; $(MAKE) $(ENVPARMS) $(CCPARMS)
E 2
#
first:
	if [ ! -d $(BINDIR) ] ; then mkdir $(BINDIR) ; fi 
	if [ ! -d $(LIBDIR) ] ; then mkdir $(LIBDIR) ; fi 
	if [ ! -d $(INCDIR) ] ; then mkdir $(INCDIR) ; fi 
#
install: libinstall
D 2
	cd $(BASE)/ar/m32; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cd $(BASE)/comp2/m32; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cd $(BASE)/cpp/m32; $(MAKE) $(ENVPARMS) $(CPPARMS) $(DIRPARMS) install
	cd $(BASE)/cmd/m32; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cd $(BASE)/optim/m32; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cd $(BASE)/as/m32; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cd $(BASE)/ld/m32; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cd $(BASE)/compress/m32; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cd $(BASE)/convert/m32; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cd $(BASE)/dis/m32; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cd $(BASE)/dump/m32; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cd $(BASE)/list/m32; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cd $(BASE)/lorder/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) install
	cd $(BASE)/mkshlib/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) install
	cd $(BASE)/nm/m32; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cd $(BASE)/size/m32; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cd $(BASE)/strip/m32; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cd $(BASE)/unix_conv/m32; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cp $(BASE)/../make/make $(BINDIR)/m32make
	cp $(BASE)/xenv/m32/env-m32 $(BINDIR)/$(SGS)env
	cp $(BASE)/xenv/m32/make.tmp $(BINDIR)/make
	cp $(BASE)/xenv/m32/vax $(BINDIR)
	cp $(BASE)/xenv/m32/u3b $(BINDIR)
	cp $(BASE)/xenv/m32/u370 $(BINDIR)
	cp $(BASE)/xenv/m32/u3b15 $(BINDIR)
	cp $(BASE)/xenv/m32/u3b5 $(BINDIR)
	cp $(BASE)/xenv/m32/u3b2 $(BINDIR)
E 2
I 2
	cd $(BASE)/ar/m68k; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cd $(BASE)/comp2/m68k; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cd $(BASE)/cpp/m68k; $(MAKE) $(ENVPARMS) $(CPPARMS) $(DIRPARMS) install
	cd $(BASE)/cmd/m68k; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cd $(BASE)/optim/m68k; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cd $(BASE)/hloptim/m68k; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cd $(BASE)/as/m68k; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cd $(BASE)/ld/m68k; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cd $(BASE)/compress/m68k; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cd $(BASE)/convert/m68k; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cd $(BASE)/dis/m68k; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cd $(BASE)/dump/m68k; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cd $(BASE)/list/m68k; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cd $(BASE)/lorder/m68k; $(MAKE) $(ENVPARMS) $(DIRPARMS) install
	cd $(BASE)/mkshlib/m68k; $(MAKE) $(ENVPARMS) $(DIRPARMS) install
	cd $(BASE)/nm/m68k; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cd $(BASE)/size/m68k; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cd $(BASE)/strip/m68k; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
	cd $(BASE)/unix_conv/m68k; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
E 2
#
libinstall:
D 2
	cd $(BASE)/libld/m32; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
E 2
I 2
	cd $(BASE)/libld/m68k; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) install
E 2
#
save:
D 2
	cd $(BASE)/inc/m32; $(MAKE) $(ENVPARMS) INCDIR=$(INCDIR) save
	cd $(BASE)/libld/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) save
	cd $(BASE)/ar/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) save
	cd $(BASE)/comp2/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) save
	cd $(BASE)/cpp/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) save
	cd $(BASE)/cmd/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) save
	cd $(BASE)/optim/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) save
	cd $(BASE)/as/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) save
	cd $(BASE)/ld/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) save
	cd $(BASE)/compress/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) save
	cd $(BASE)/convert/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) save
	cd $(BASE)/dis/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) save
	cd $(BASE)/dump/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) save
	cd $(BASE)/list/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) save
	cd $(BASE)/lorder/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) save
	cd $(BASE)/mkshlib/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) save
	cd $(BASE)/nm/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) save
	cd $(BASE)/size/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) save
	cd $(BASE)/strip/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) save
	cd $(BASE)/unix_conv/m32; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) save
	cd $(BASE)/../make; /bin/make -f make.mk $(DIRPARMS) save
E 2
I 2
	cd $(BASE)/inc/m68k; $(MAKE) $(ENVPARMS) INCDIR=$(INCDIR) save
	cd $(BASE)/libld/m68k; $(MAKE) $(ENVPARMS) $(DIRPARMS) save
	cd $(BASE)/ar/m68k; $(MAKE) $(ENVPARMS) $(DIRPARMS) save
	cd $(BASE)/comp2/m68k; $(MAKE) $(ENVPARMS) $(DIRPARMS) save
	cd $(BASE)/cpp/m68k; $(MAKE) $(ENVPARMS) $(DIRPARMS) save
	cd $(BASE)/cmd/m68k; $(MAKE) $(ENVPARMS) $(DIRPARMS) save
	cd $(BASE)/optim/m68k; $(MAKE) $(ENVPARMS) $(DIRPARMS) save
	cd $(BASE)/hloptim/m68k; $(MAKE) $(ENVPARMS) $(DIRPARMS) save
	cd $(BASE)/as/m68k; $(MAKE) $(ENVPARMS) $(DIRPARMS) save
	cd $(BASE)/ld/m68k; $(MAKE) $(ENVPARMS) $(DIRPARMS) save
	cd $(BASE)/compress/m68k; $(MAKE) $(ENVPARMS) $(DIRPARMS) save
	cd $(BASE)/convert/m68k; $(MAKE) $(ENVPARMS) $(DIRPARMS) save
	cd $(BASE)/dis/m68k; $(MAKE) $(ENVPARMS) $(DIRPARMS) save
	cd $(BASE)/dump/m68k; $(MAKE) $(ENVPARMS) $(DIRPARMS) save
	cd $(BASE)/list/m68k; $(MAKE) $(ENVPARMS) $(DIRPARMS) save
	cd $(BASE)/lorder/m68k; $(MAKE) $(ENVPARMS) $(DIRPARMS) save
	cd $(BASE)/mkshlib/m68k; $(MAKE) $(ENVPARMS) $(DIRPARMS) save
	cd $(BASE)/nm/m68k; $(MAKE) $(ENVPARMS) $(DIRPARMS) save
	cd $(BASE)/size/m68k; $(MAKE) $(ENVPARMS) $(DIRPARMS) save
	cd $(BASE)/strip/m68k; $(MAKE) $(ENVPARMS) $(DIRPARMS) save
	cd $(BASE)/unix_conv/m68k; $(MAKE) $(ENVPARMS) $(CCPARMS) $(DIRPARMS) save
E 2
#
uninstall:
D 2
	cd $(BASE)/inc/m32; $(MAKE) $(ENVPARMS) INCDIR=$(INCDIR) uninstall
	cd $(BASE)/libld/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) uninstall
	cd $(BASE)/ar/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) uninstall
	cd $(BASE)/comp2/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) uninstall
	cd $(BASE)/cpp/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) uninstall
	cd $(BASE)/cmd/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) uninstall
	cd $(BASE)/optim/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) uninstall
	cd $(BASE)/as/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) uninstall
	cd $(BASE)/ld/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) uninstall
	cd $(BASE)/compress/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) uninstall
	cd $(BASE)/convert/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) uninstall
	cd $(BASE)/dis/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) uninstall
	cd $(BASE)/dump/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) uninstall
	cd $(BASE)/list/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) uninstall
	cd $(BASE)/lorder/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) uninstall
	cd $(BASE)/mkshlib/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) uninstall
	cd $(BASE)/nm/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) uninstall
	cd $(BASE)/size/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) uninstall
	cd $(BASE)/strip/m32; $(MAKE) $(ENVPARMS) $(DIRPARMS) uninstall
	cd $(BASE)/../make; /bin/make -f make.mk $(DIRPARMS) uninstall
E 2
I 2
	cd $(BASE)/inc/m68k; $(MAKE) $(ENVPARMS) INCDIR=$(INCDIR) uninstall
	cd $(BASE)/libld/m68k; $(MAKE) $(ENVPARMS) $(DIRPARMS) uninstall
	cd $(BASE)/ar/m68k; $(MAKE) $(ENVPARMS) $(DIRPARMS) uninstall
	cd $(BASE)/comp2/m68k; $(MAKE) $(ENVPARMS) $(DIRPARMS) uninstall
	cd $(BASE)/cpp/m68k; $(MAKE) $(ENVPARMS) $(DIRPARMS) uninstall
	cd $(BASE)/cmd/m68k; $(MAKE) $(ENVPARMS) $(DIRPARMS) uninstall
	cd $(BASE)/optim/m68k; $(MAKE) $(ENVPARMS) $(DIRPARMS) uninstall
	cd $(BASE)/hloptim/m68k; $(MAKE) $(ENVPARMS) $(DIRPARMS) uninstall
	cd $(BASE)/as/m68k; $(MAKE) $(ENVPARMS) $(DIRPARMS) uninstall
	cd $(BASE)/ld/m68k; $(MAKE) $(ENVPARMS) $(DIRPARMS) uninstall
	cd $(BASE)/compress/m68k; $(MAKE) $(ENVPARMS) $(DIRPARMS) uninstall
	cd $(BASE)/convert/m68k; $(MAKE) $(ENVPARMS) $(DIRPARMS) uninstall
	cd $(BASE)/dis/m68k; $(MAKE) $(ENVPARMS) $(DIRPARMS) uninstall
	cd $(BASE)/dump/m68k; $(MAKE) $(ENVPARMS) $(DIRPARMS) uninstall
	cd $(BASE)/list/m68k; $(MAKE) $(ENVPARMS) $(DIRPARMS) uninstall
	cd $(BASE)/lorder/m68k; $(MAKE) $(ENVPARMS) $(DIRPARMS) uninstall
	cd $(BASE)/mkshlib/m68k; $(MAKE) $(ENVPARMS) $(DIRPARMS) uninstall
	cd $(BASE)/nm/m68k; $(MAKE) $(ENVPARMS) $(DIRPARMS) uninstall
	cd $(BASE)/size/m68k; $(MAKE) $(ENVPARMS) $(DIRPARMS) uninstall
	cd $(BASE)/strip/m68k; $(MAKE) $(ENVPARMS) $(DIRPARMS) uninstall
E 2
#
shrink: libshrink
D 2
	cd $(BASE)/ar/m32; $(MAKE) $(ENVPARMS) $(CCPARMS) shrink
	cd $(BASE)/comp2/m32; $(MAKE) $(ENVPARMS) shrink
	cd $(BASE)/cpp/m32; $(MAKE) $(ENVPARMS) shrink
	cd $(BASE)/cmd/m32; $(MAKE) $(ENVPARMS) shrink
	cd $(BASE)/optim/m32; $(MAKE) $(ENVPARMS) shrink
	cd $(BASE)/as/m32; $(MAKE) $(ENVPARMS) shrink
	cd $(BASE)/ld/m32; $(MAKE) $(ENVPARMS) shrink
	cd $(BASE)/compress/m32; $(MAKE) $(ENVPARMS) shrink
	cd $(BASE)/convert/m32; $(MAKE) $(ENVPARMS) shrink
	cd $(BASE)/dis/m32; $(MAKE) $(ENVPARMS) shrink
	cd $(BASE)/dump/m32; $(MAKE) $(ENVPARMS) shrink
	cd $(BASE)/list/m32; $(MAKE) $(ENVPARMS) shrink
	cd $(BASE)/lorder/m32; $(MAKE) $(ENVPARMS) shrink
	cd $(BASE)/mkshlib/m32; $(MAKE) $(ENVPARMS) shrink
	cd $(BASE)/nm/m32; $(MAKE) $(ENVPARMS) shrink
	cd $(BASE)/size/m32; $(MAKE) $(ENVPARMS) shrink
	cd $(BASE)/strip/m32; $(MAKE) $(ENVPARMS) shrink
	cd $(BASE)/unix_conv/m32; $(MAKE) $(ENVPARMS) $(CCPARMS) shrink
	cd $(BASE)/../make; /bin/make -f make.mk clobber
E 2
I 2
	cd $(BASE)/ar/m68k; $(MAKE) $(ENVPARMS) $(CCPARMS) shrink
	cd $(BASE)/comp2/m68k; $(MAKE) $(ENVPARMS) shrink
	cd $(BASE)/cpp/m68k; $(MAKE) $(ENVPARMS) shrink
	cd $(BASE)/cmd/m68k; $(MAKE) $(ENVPARMS) shrink
	cd $(BASE)/optim/m68k; $(MAKE) $(ENVPARMS) shrink
	cd $(BASE)/hloptim/m68k; $(MAKE) $(ENVPARMS) shrink
	cd $(BASE)/as/m68k; $(MAKE) $(ENVPARMS) shrink
	cd $(BASE)/ld/m68k; $(MAKE) $(ENVPARMS) shrink
	cd $(BASE)/compress/m68k; $(MAKE) $(ENVPARMS) shrink
	cd $(BASE)/convert/m68k; $(MAKE) $(ENVPARMS) shrink
	cd $(BASE)/dis/m68k; $(MAKE) $(ENVPARMS) shrink
	cd $(BASE)/dump/m68k; $(MAKE) $(ENVPARMS) shrink
	cd $(BASE)/list/m68k; $(MAKE) $(ENVPARMS) shrink
	cd $(BASE)/lorder/m68k; $(MAKE) $(ENVPARMS) shrink
	cd $(BASE)/mkshlib/m68k; $(MAKE) $(ENVPARMS) shrink
	cd $(BASE)/nm/m68k; $(MAKE) $(ENVPARMS) shrink
	cd $(BASE)/size/m68k; $(MAKE) $(ENVPARMS) shrink
	cd $(BASE)/strip/m68k; $(MAKE) $(ENVPARMS) shrink
	cd $(BASE)/unix_conv/m68k; $(MAKE) $(ENVPARMS) $(CCPARMS) shrink
E 2
#
libshrink:
D 2
	cd $(BASE)/libld/m32; $(MAKE) $(ENVPARMS) shrink
E 2
I 2
	cd $(BASE)/libld/m68k; $(MAKE) $(ENVPARMS) shrink
E 2
E 1
