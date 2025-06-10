#	@(#)libm881u.mk	7.1	
#	@(#)libm881.mk	6.2	
#	@(#)libm.mk	6.1 
#
# makefile for libm881u
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
LIBNAME = lib$(VARIANT)m881u.a
LIBNAM=libtemp.a
AR=ar
CFLAGS=-c -O
FP=m68881u		# generate 881 code, use "u"niversal return value
#			# (return float/double in both %fp0 and %d0/%d1)
CC=	FP=$(FP) cc
DBLALIGN=NO
FILES =\
	$(LIBNAM)(asin.o)\
	$(LIBNAM)(atan.o)\
	$(LIBNAM)(erf.o)\
	$(LIBNAM)(fabs.o)\
	$(LIBNAM)(floor.o)\
	$(LIBNAM)(fmod.o)\
	$(LIBNAM)(gamma.o)\
	$(LIBNAM)(hypot.o)\
	$(LIBNAM)(jn.o)\
	$(LIBNAM)(j0.o)\
	$(LIBNAM)(j1.o)\
	$(LIBNAM)(pow.o)\
	$(LIBNAM)(log.o)\
	$(LIBNAM)(sin.o)\
	$(LIBNAM)(sqrt.o)\
	$(LIBNAM)(tan.o)\
	$(LIBNAM)(tanh.o)\
	$(LIBNAM)(sinh.o)\
	$(LIBNAM)(exp.o)\
	$(LIBNAM)(matherr.o)\
	$(LIBNAM)(asin881.o)\
	$(LIBNAM)(atan881.o)\
	$(LIBNAM)(etox881.o)\
	$(LIBNAM)(log881.o)\
	$(LIBNAM)(mul881.o)\
	$(LIBNAM)(nan881.o)\
	$(LIBNAM)(sin881.o)\
	$(LIBNAM)(sinh881.o)\
	$(LIBNAM)(sqrt881.o)\
	$(LIBNAM)(tan881.o)\
	$(LIBNAM)(tanh881.o)

.c.a:
	$(CC) $(CFLAGS) $*.c
	chmod $(PROT) $*.o
	chgrp $(GRP)  $*.o
	chown $(OWN)  $*.o
	$(AR) rv $@ $*.o
	rm $*.o

.s.a:
	$(AS) $(ASFLAGS) -o $*.o $*.s
	chmod $(PROT) $*.o
	chgrp $(GRP)  $*.o
	chown $(OWN)  $*.o
	$(AR) rv $@ $*.o
	rm $*.o

dolink:
	rm -f *.[cs]
	ln ../libm881/*.[cs] .

all: normallib profiledlib

normallib:
	make -e -f libm881u.mk $(LIBNAME)

libtemp.a: $(FILES)

$(LIBNAME): dolink
	-cp $(LIBNAME) libtemp.a # start with old lib, if it exists.
	make -e -f libm881u.mk libtemp.a
	mv libtemp.a $(LIBNAME)

$(FILES): $(INCROOT)/usr/include/math.h $(INCROOT)/usr/include/values.h \
	$(INCROOT)/usr/include/errno.h $(INCROOT)/usr/include/sys/errno.h \
	$(INCROOT)/usr/include/sys/fpu.h

profiledlib:
	make -f libm881u.mk -e p.$(LIBNAME) LIBNAME=p.$(LIBNAME) \
						CFLAGS="-p $(CFLAGS)"

mq: mq.o $(LIBNAME)
	$(CC) $(CFLAGS) mq.o $(LIBNAME) -o mq

mq.o: asin.test.h atan.test.h constants.h exp.test.h gamma.test.h \
	log.test.h machar.h pow.test.h prnt.env.h sin.test.h sinh.test.h \
	sqrt.test.h tan.test.h tanh.test.h tests.h 
	$(CC) $(CFLAGS) -c mq.c

install: all save move

clean:
	-rm -f *.o

clobber:	clean
	-rm -f $(LIBNAME) p.$(LIBNAME)

move:
	rm -f $(LIB)/$(LIBNAME)
	cp $(LIBNAME) $(LIB)/$(LIBNAME) ; rm -f $(LIBNAME)
	chmod $(PROT) $(LIB)/$(LIBNAME)
	chmod -x $(LIB)/$(LIBNAME)	# not executable
	chgrp $(GRP) $(LIB)/$(LIBNAME)
	chown $(OWN) $(LIB)/$(LIBNAME)

	rm -f $(LIBP)/$(LIBNAME)
	cp p.$(LIBNAME) $(LIBP)/$(LIBNAME) ; rm -f p.$(LIBNAME)
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
