#ident	"@(#)xenv:sgs.mk	1.3"
#	m68k/3b15/3b5/3b2 Cross-SGS Global Makefile
#	PATHEDIT MUST BE RUN BEFORE THIS MAKEFILE IS USED!
#
#			      W A R N I N G
#
#	Do not directly edit this file unless you are editing the
#	master copy under <somewhere>/src/cmd/sgs/xenv!  Otherwise
#	your changes will be lost when the next pathedit is run.
#

BINDIR=
SGS=

#(fnf)all:	install libs
all:		sgs
libs:
	cd ../../lib; $(BINDIR)/$(SGS)env make -f lib.mk install
	cd xenv/m68k; \
		$(BINDIR)/$(SGS)env make libshrink; \
		$(BINDIR)/$(SGS)env make libinstall; \
		$(BINDIR)/$(SGS)env make libshrink;

install:  sgs
	cd xenv/m68k; make install

first:
	cd xenv/m68k; make first

sgs:	
	cd xenv/m68k; $(MAKE) all

uninstall:
	cd xenv/m68k; make uninstall

save:
	cd xenv/m68k; make save

shrink:	clobber
	if [ -f "$(BINDIR)/$(SGS)env" ] ; \
	then \
		cd ../../lib; $(BINDIR)/$(SGS)env make -f lib.mk clobber ; \
	fi

clobber:
	cd xenv/m68k; make shrink
