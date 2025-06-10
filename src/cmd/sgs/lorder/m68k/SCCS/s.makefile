h27111
s 00003/00003/00043
d D 1.2 86/09/17 16:28:46 fnf 2 1
c Changes for release integration.
e
s 00046/00000/00000
d D 1.1 86/09/17 15:14:06 fnf 1 0
c From Unix 5.3 release for 3b2
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
#ident	"@(#)lorder:m32/makefile	1.8"
E 2
I 2
#ident	"@(#)lorder:m68k/makefile	1.8"
E 2
#
#	BELLMAC-32 LORDER MAKEFILE
#
#
D 2
SGS=m32
E 2
I 2
SGS=m68k
E 2
OWN=bin
GRP=bin
SED=/bin/sed
SEDFLAGS=-e
SED_CMD=$(SED) $(SEDFLAGS)
#
#	Internal Directories
#
BASE= ../..
D 2
BINDIR= $(BASE)/xenv/m32/bin
E 2
I 2
BINDIR= $(BASE)/xenv/m68k/bin
E 2
COMLORDER= $(BASE)/lorder/common
#
all:		lorder
#
lorder:		$(COMLORDER)/lorder
		-rm -f lorder
		$(SED_CMD) "s/SGS/$(SGS)/" < $(COMLORDER)/lorder > lorder
#
install:	$(BINDIR)/$(SGS)lorder
#
$(BINDIR)/$(SGS)lorder:	lorder
		sh $(BASE)/sgs.install 755 $(OWN) $(GRP) $(BINDIR)/$(SGS)lorder lorder
#
save:		$(BINDIR)/$(SGS)lorder
		-rm -f $(BINDIR)/$(SGS)lorder.back
		cp $(BINDIR)/$(SGS)lorder $(BINDIR)/$(SGS)lorder.back
#
uninstall:	$(BINDIR)/$(SGS)lorder.back
		-rm -f $(BINDIR)/$(SGS)lorder
		cp $(BINDIR)/$(SGS)lorder.back $(BINDIR)/$(SGS)lorder
#
shrink:		
		-rm -f lorder
E 1
