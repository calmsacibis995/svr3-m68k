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

SGS=

#(fnf)all:	install libs
all:		sgs
libs:
	cd ../../lib; make -f lib.mk install

install:  sgs
	cd xenv/m68k; make BINDIR=$(ROOT)/bin LIBDIR=$(ROOT)/lib install

first:
	cd xenv/m68k; make first

sgs:	
	cd xenv/m68k; $(MAKE) all

uninstall:
	cd xenv/m68k; make uninstall

save:
	cd xenv/m68k; make save

shrink:	clobber
	if [ true ] ; \
	then \
		cd ../../lib; make -f lib.mk clobber ; \
	fi

clobber:
	cd xenv/m68k; make shrink
