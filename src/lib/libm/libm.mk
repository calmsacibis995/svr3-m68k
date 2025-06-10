#	@(#)libm.mk	6.1 
#
# makefile for libm
#
# Note that the object list given in FILES is in the order in which the files
# should be added to the archive when it is created.  If the files are not
# ordered correctly, linking of libm fails on the pdp11 and is slowed down
# on other machines.

OWN=	bin		#file owner
GRP=	bin		#file group
PROT=	664		#protections

ROOT=					# default root directory
INCROOT=				# default include parent directory

LIB=$(ROOT)/lib
LIBP=$(ROOT)/usr/lib/libp
VARIANT =
LIBNAME = lib$(VARIANT)m.a
AR=ar
FILES =\
	$(LIBNAME)(asin.o)\
	$(LIBNAME)(atan.o)\
	$(LIBNAME)(erf.o)\
	$(LIBNAME)(fabs.o)\
	$(LIBNAME)(floor.o)\
	$(LIBNAME)(fmod.o)\
	$(LIBNAME)(gamma.o)\
	$(LIBNAME)(hypot.o)\
	$(LIBNAME)(jn.o)\
	$(LIBNAME)(j0.o)\
	$(LIBNAME)(j1.o)\
	$(LIBNAME)(pow.o)\
	$(LIBNAME)(log.o)\
	$(LIBNAME)(sin.o)\
	$(LIBNAME)(sqrt.o)\
	$(LIBNAME)(tan.o)\
	$(LIBNAME)(tanh.o)\
	$(LIBNAME)(sinh.o)\
	$(LIBNAME)(exp.o)\
	$(LIBNAME)(matherr.o)

all: $(LIBNAME) profiledlib

$(LIBNAME): $(FILES)
	$(CC) -c $(CFLAGS) $(?:.o=.c)
	chmod $(PROT) $?
	chgrp $(GRP)  $?
	chown $(OWN)  $?
	$(AR) rv $(LIBNAME) $?
	rm $?

$(FILES): $(INCROOT)/usr/include/math.h $(INCROOT)/usr/include/values.h \
	$(INCROOT)/usr/include/errno.h $(INCROOT)/usr/include/sys/errno.h

.c.a:;

profiledlib:
	make -f libm.mk -e libp.$(LIBNAME) LIBNAME=libp.$(LIBNAME) \
						CFLAGS="-p $(CFLAGS)"

mq: mq.o $(LIBNAME)
	$(CC) $(CFLAGS) mq.o $(LIBNAME) -o mq

mq.o: asin.test.h atan.test.h constants.h exp.test.h gamma.test.h \
	log.test.h machar.h pow.test.h prnt.env.h sin.test.h sinh.test.h \
	sqrt.test.h tan.test.h tanh.test.h tests.h \

	$(CC) $(CFLAGS) -c mq.c

install: all save move

clean:
	-rm -f *.o

clobber:	clean
	-rm -f $(LIBNAME) libp.$(LIBNAME)

move:
	rm -f $(LIB)/$(LIBNAME)
	cp $(LIBNAME) $(LIB)/$(LIBNAME) ; rm -f $(LIBNAME)
	chmod $(PROT) $(LIB)/$(LIBNAME)
	chmod -x $(LIB)/$(LIBNAME)	# not executable
	chgrp $(GRP) $(LIB)/$(LIBNAME)
	chown $(OWN) $(LIB)/$(LIBNAME)

	rm -f $(LIBP)/$(LIBNAME)
	cp libp.$(LIBNAME) $(LIBP)/$(LIBNAME) ; rm -f libp.$(LIBNAME)
	chmod $(PROT) $(LIBP)/$(LIBNAME)
	chmod -x $(LIBP)/$(LIBNAME)	# not executable
	chgrp $(GRP) $(LIBP)/$(LIBNAME)
	chown $(OWN) $(LIBP)/$(LIBNAME)

#--------------------------

uninstall:	$(LIB)/OLD$(LIBNAME) $(LIBP)/OLD$(LIBNAME)
		
		mv -f $(LIB)/OLD$(LIBNAME) $(LIB)/$(LIBNAME)lib$(VARIANT)c.a
		mv -f $(LIBP)/OLD$(LIBNAME) $(LIBP)/$(LIBNAME)

#--------------------------

save:		
	if test -f $(LIB)/$(LIBNAME) ; \
	then \
		rm -f $(LIB)/OLD$(LIBNAME) ; \
		cp $(LIB)/$(LIBNAME) $(LIB)/OLD$(LIBNAME) ; \
	fi

	if test -f $(LIBP)/$(LIBNAME) ; \
	then \
		rm -f $(LIBP)/OLD$(LIBNAME) ; \
		cp $(LIBP)/$(LIBNAME) $(LIBP)/OLD$(LIBNAME) ; \
	fi
