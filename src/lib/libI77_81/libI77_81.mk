#	@(#)libI77_81.mk	7.1 
#	@(#)libI77.mk	7.1 

OWN=	bin		#file owner
GRP=	bin		#file group
PROT=	664		#protections

ROOT = 
INCROOT =
TESTDIR = .
LIBNAME = libI77_81.a
TESTLIB = $(TESTDIR)/$(LIBNAME)
INSDIR = $(ROOT)/usr/lib
FRC =

all:	$(TESTLIB)

$(TESTLIB):
	(cd ../libI77; make -f libI77.mk ./libI77.a FP=m68881 TESTDIR=.)
	mv ../libI77/libI77.a $(TESTLIB)

install:	all save
		rm -f $(INSDIR)/$(LIBNAME)
		cp $(TESTLIB) $(INSDIR)/$(LIBNAME)
		chmod $(PROT) $(INSDIR)/$(LIBNAME)
		chmod -x $(INSDIR)/$(LIBNAME)
		chgrp $(GRP) $(INSDIR)/$(LIBNAME)
		chown $(OWN) $(INSDIR)/$(LIBNAME)

#--------------------------

uninstall:	$(INSDIR)/OLD$(LIBNAME)
		-rm -f $(INSDIR)/$(LIBNAME)
		cp $(INSDIR)/OLD$(LIBNAME) $(INSDIR)/$(LIBNAME)
		chmod $(PROT) $(INSDIR)/$(LIBNAME)
		chmod -x $(INSDIR)/$(LIBNAME)
		chgrp $(GRP) $(INSDIR)/$(LIBNAME)
		chown $(OWN) $(INSDIR)/$(LIBNAME)

#--------------------------

save:		
		if test -f $(INSDIR)/$(LIBNAME) ; \
		then \
			rm -f $(INSDIR)/OLD$(LIBNAME) ; \
			cp $(INSDIR)/$(LIBNAME) $(INSDIR)/OLD$(LIBNAME) ; \
		fi

clean:
	(cd ../libI77; make -f libI77.mk clean)

clobber:
	(cd ../libI77; make -f libI77.mk clobber)
	-rm -f $(TESTLIB)

FRC:
