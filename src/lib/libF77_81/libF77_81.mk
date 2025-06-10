#	@(#)libF77_81.mk	7.1 
#	@(#)libF77.mk	7.1 

OWN=	bin		#file owner
GRP=	bin		#file group
PROT=	664		#protections

ROOT =
INSDIR = $(ROOT)/usr/lib
TESTDIR = .
FRC =

all: libF77_81.a

libF77_81.a :
	(cd ../libF77; make -f libF77.mk libF77.a FP=m68881)
	mv ../libF77/libF77.a $(TESTDIR)/libF77_81.a

install: all save
	-rm -f $(INSDIR)/libF77_81.a
	cp $(TESTDIR)/libF77_81.a $(INSDIR)/libF77_81.a 
	chmod $(PROT) $(INSDIR)/libF77_81.a
	chmod -x $(INSDIR)/libF77_81.a
	chgrp $(GRP) $(INSDIR)/libF77_81.a
	chown $(OWN) $(INSDIR)/libF77_81.a

#--------------------------

uninstall:	$(INSDIR)/OLDlibF77_81.a
		-rm -f $(INSDIR)/libF77_81.a
		cp $(INSDIR)/OLDlibF77_81.a $(INSDIR)/libF77_81.a
		chmod $(PROT) $(INSDIR)/libF77_81.a
		chmod -x $(INSDIR)/libF77_81.a
		chgrp $(GRP) $(INSDIR)/libF77_81.a
		chown $(OWN) $(INSDIR)/libF77_81.a

#--------------------------

save:		
		if test -f $(INSDIR)/libF77_81.a ; \
		then \
			rm -f $(INSDIR)/OLDlibF77_81.a ; \
			cp $(INSDIR)/libF77_81.a $(INSDIR)/OLDlibF77_81.a ; \
		fi

clean:
	(cd ../libF77; make -f libF77.mk clean)

clobber:
	(cd ../libF77; make -f libF77.mk clobber)
	 -rm -f $(TESTDIR)/libF77_81.a

FRC:

