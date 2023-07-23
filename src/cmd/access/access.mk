#		Copyright (c) 1985 Motorola, Inc.   #
#		All Rights Reserved		#
#	@(#)makefile	1.3	10/3/85

ROOT = 
BIN = $(ROOT)/bin
LIB = $(ROOT)/usr/lib
LIBNAME = $(LIB)/libaccess.a
LDFLAGS = -laccess
LIBSRC = $(ROOT)/usr/src/lib/libaccess
INCLD=$(ROOT)/usr/include
FRC = 
PERMSDIR = $(ROOT)/etc
PERMSFILE = perms


CFLAGS =  -O  -I$(INCLD) 

RUTILS =\
	dcpy\
	fmt\
	fs\
	mnt\
	chk\
	tt\
	umnt\
	$(FRC)

UTILS =\
	real\
	$(FRC)

all:	$(RUTILS) $(UTILS)

$(LIBNAME):	
	cd $(LIBSRC)   ;  $(MAKE)

$(INCLD)/access.h:	
	cd $(ROOT)/usr/src/head ;  echo access.h | cpio -pdvm $(INCLD)

dcpy:	dcpy.c $(INCLD)/access.h
	cc $(CFLAGS) -o dcpy dcpy.c $(LDFLAGS)

fmt:	fmt.c $(INCLD)/access.h
	cc $(CFLAGS) -o fmt fmt.c $(LDFLAGS)

fs:	fs.c $(INCLD)/access.h
	cc $(CFLAGS) -o fs fs.c $(LDFLAGS)

mnt:	mnt.c $(INCLD)/access.h
	cc $(CFLAGS) -o mnt mnt.c $(LDFLAGS)

chk:	chk.c $(INCLD)/access.h
	cc $(CFLAGS) -o chk chk.c $(LDFLAGS)

real:	real.c $(INCLD)/access.h
	cc $(CFLAGS) -o real real.c $(LDFLAGS)

tt:	tt.c $(INCLD)/access.h
	cc $(CFLAGS) -o tt tt.c $(LDFLAGS)

umnt:	umnt.c $(INCLD)/access.h
	cc $(CFLAGS) -o umnt umnt.c $(LDFLAGS)

#$(PERMSFILE):	$(PERMSFILE).m4	
	m4 -D$(SYSTEM) $(PERMSFILE).m4 > $(PERMSFILE)

# perms file should be installed be /usr/src/cmd/.adm/.adm.mk

# installperms:	$(PERMSFILE)
#	-/etc/install   -f $(PERMSDIR) -o  $(PERMSFILE)

install:	all 
	for i in $(RUTILS) ;\
	do \
	/usr/bin/cpset $$i     $(BIN)   4755 root bin ;\
	done
	for i in $(UTILS) ;\
	do \
	/usr/bin/cpset $$i     $(BIN)   0755 bin bin ;\
	done

clean:
	-rm -f *.o $(RUTILS) $(UTILS) $(PERMSFILE)

clobber: clean
