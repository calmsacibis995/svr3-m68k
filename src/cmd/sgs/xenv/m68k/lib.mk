#ident	"@(#)xenv:lib.mk	1.4"
#	Global Library Makefile
#
#			      W A R N I N G
#
#	Do not directly edit this file unless you are editing the
#	master copy under <somewhere>/src/cmd/sgs/xenv!  Otherwise
#	your changes will be lost when the next pathedit is run.
#
#
LIB=$(ROOT)/lib
USRLIB=$(ROOT)/usr/lib
LIBP=$(ROOT)/usr/lib/libp
SHLIB=$(ROOT)/shlib
LIBLIST=*
MAKE=make
SGS=

all:
	for i in $(LIBLIST); \
	do \
	   if [ -d $$i ]; \
	   then \
	   	cd $$i; $(MAKE) -f $$i.mk; cd ..; \
	   fi \
	done

install:
	for i in $(LIBLIST); \
	do \
	   if [ -d $$i ]; \
	   then \
	   	cd $$i; \
	   	$(MAKE) -f $$i.mk LIB=$(LIB) LIBP=$(LIBP) USRLIB=$(USRLIB) SGS=$(SGS) SHLIB=$(SHLIB) install; \
	   	cd ..; \
	   fi \
	done

clean:
	for i in $(LIBLIST); \
	do \
	   if [ -d $$i ]; \
	   then \
	   	cd $$i; $(MAKE) -f $$i.mk clean; cd ..; \
	   fi \
	done

clobber:
	for i in $(LIBLIST); \
	do \
	   if [ -d $$i ]; \
	   then \
	   	cd $$i; $(MAKE) -f $$i.mk clobber; cd ..; \
	   fi \
	done
