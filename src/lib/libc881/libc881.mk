#	@(#)libc881.mk	6.2	
#	@(#)libc.mk	6.1	
#
# makefile for libc881
#
#
# The variable PROF is null by default, causing both the standard C library
# and a profiled library to be maintained.  If profiled object is not 
# desired, the reassignment PROF=@# should appear in the make command line.
#
# The variable IGN may be set to -i by the assignment IGN=-i in order to
# allow a make to complete even if there are compile errors in individual
# modules.
#
# See also the comments in the lower-level machine-dependent makefiles.
#
OWN=	bin		#file owner
GRP=	bin		#file group
PROT=	664		#protections

FP=m68881		# flag used by C compiler to generate 881 code
DBLALIGN=NO
VARIANT=
CFLAGS=-O
PCFLAGS=
PFX=
CC=$(PFX)cc
AR=$(PFX)ar
STRIP=$(PFX)strip
LORDER=$(PFX)lorder
ROOT=					# default root directory. This include
#					 the include directories.
LIB=$(ROOT)/lib
LIBP=$(ROOT)/usr/lib/libp

PROF=
DONE=

all:
	if m68k ; then make -e -f  libc881.mk specific MACHINE=m68k ; fi

specific:
	# PROCESSOR = $(PROCESSOR)
	# DBLALIGN  = $(DBLALIGN)
	# FP        = $(FP)
	#
	# compile portable library modules
	cd ../libc/port; make -e CC=$(CC) FP=$(FP) DBLALIGN=$(DBLALIGN) 
	#
	# compile machine-dependent library modules
	cd ../libc/$(MACHINE); make -e all CC=$(CC) FP=$(FP) DBLALIGN=$(DBLALIGN) 
	# compile MC68881-dependent library modules
	cd m68k; make -e all CC=$(CC) FP=$(FP) DBLALIGN=$(DBLALIGN)   
	#
	# place portable modules in "object" directory, then overlay
	# 	the machine-dependent modules.
	#
	# The following two commands had to be changed as the command lines
	# became to long.  The replacement command lines follow, uncommented.
	# cp ../libc/port/*/*.[op] object
	# cp ../libc/$(MACHINE)/*/*.[op] object
	#
	-rm -rf object
	mkdir object
	cp ../libc/port/[a-p]*/*.o object
	cp ../libc/port/[q-z]*/*.o object
	$(PROF) cp ../libc/port/[a-p]*/*.p object
	$(PROF) cp ../libc/port/[q-z]*/*.p object
	cp ../libc/$(MACHINE)/*/*.o object
	$(PROF) cp ../libc/$(MACHINE)/*/*.p object
	#
	# Remove a few files made obsolete by the inclusion of MC68881-
	# specific routines.
	#
	(cd object; rm -f table.o addmag.o submag.o flsub.o ltof.o ultof.o)
	#
	# Copy MC68881-specific routines over previous versions of these
	# files.
	#
	cp m68k/*/*.[op] object
	#
	# delete temporary libraries
	-rm -f lib.libc
	$(PROF)-rm -f libp.libc
	#
	# set aside run-time modules, which don't go in library archive!
	cd object; for i in *crt0.o ; do mv $$i .. ; done
	#
	# build archive out of the remaining modules.
	cd object; make -e -f ../$(MACHINE)/makefile archive \
		AR=$(AR) STRIP=$(STRIP) LORDER=$(LORDER) PROF=$(PROF)
	-rm -rf object
	#
	$(DONE)

move:
	#
	# move the library or libraries into the correct directory
	for i in *crt0.o ; \
		do rm -f $(LIB)/$(VARIANT)$$i ; \
		cp $$i $(LIB)/$(VARIANT)$$i ; \
		chmod $(PROT) $(LIB)/$(VARIANT)$$i ; \
		chmod -x $(LIB)/$(VARIANT)$$i ; \
		chgrp $(GRP) $(LIB)/$(VARIANT)$$i ; \
		chown $(OWN) $(LIB)/$(VARIANT)$$i ; \
		rm -f $$i ; \
		done

	rm -f $(LIB)/lib$(VARIANT)c881.a
	cp lib.libc $(LIB)/lib$(VARIANT)c881.a
	chmod $(PROT) $(LIB)/lib$(VARIANT)c881.a
	chmod -x $(LIB)/lib$(VARIANT)c881.a	# not executable
	chgrp $(GRP) $(LIB)/lib$(VARIANT)c881.a
	chown $(OWN) $(LIB)/lib$(VARIANT)c881.a

	$(PROF)if test -d $(LIBP) ; \
		then : ; else mkdir $(LIBP) ; fi 
	$(PROF)rm -f $(LIBP)/lib$(VARIANT)c881.a
	$(PROF)cp libp.libc $(LIBP)/lib$(VARIANT)c881.a
	$(PROF)chmod $(PROT) $(LIBP)/lib$(VARIANT)c881.a
	$(PROF)chmod -x $(LIBP)/lib$(VARIANT)c881.a	# not executable
	$(PROF)chgrp $(GRP) $(LIBP)/lib$(VARIANT)c881.a
	$(PROF)chown $(OWN) $(LIBP)/lib$(VARIANT)c881.a

install: all save move

#--------------------------

uninstall:	$(LIB)/OLDlib$(VARIANT)c881.a $(LIBP)/OLDlib$(VARIANT)c881.a \
		$(LIB)/OLD$(VARIANT)mcrt0.o $(LIB)/OLD$(VARIANT)crt0.o
		
		mv -f $(LIB)/OLDlib$(VARIANT)c881.a $(LIB)/lib$(VARIANT)c881.a
		mv -f $(LIBP)/OLDlib$(VARIANT)c881.a $(LIBP)/lib$(VARIANT)c881.a
		mv -f $(LIB)/OLD$(VARIANT)mcrt0.o $(LIB)/$(VARIANT)mcrt0.o
		mv -f $(LIB)/OLD$(VARIANT)crt0.o $(LIB)/$(VARIANT)crt0.o

#--------------------------

save:		
	if test -f $(LIB)/lib$(VARIANT)c881.a ; \
	then \
		rm -f $(LIB)/OLDlib$(VARIANT)c881.a ; \
		cp $(LIB)/lib$(VARIANT)c881.a $(LIB)/OLDlib$(VARIANT)c881.a ; \
		chmod $(PROT) $(LIB)/OLDlib$(VARIANT)c881.a ; \
		chmod -x $(LIB)/OLDlib$(VARIANT)c881.a ; \
		chgrp $(GRP) $(LIB)/OLDlib$(VARIANT)c881.a ; \
		chown $(OWN) $(LIB)/OLDlib$(VARIANT)c881.a ; \
	fi

	$(PROF)if test -f $(LIBP)/lib$(VARIANT)c881.a ; \
	then \
		rm -f $(LIBP)/OLDlib$(VARIANT)c881.a ; \
		cp $(LIBP)/lib$(VARIANT)c881.a $(LIBP)/OLDlib$(VARIANT)c881.a ;\
		chmod $(PROT) $(LIBP)/OLDlib$(VARIANT)c881.a ; \
		chmod -x $(LIBP)/OLDlib$(VARIANT)c881.a ; \
		chgrp $(GRP) $(LIBP)/OLDlib$(VARIANT)c881.a ; \
		chown $(OWN) $(LIBP)/OLDlib$(VARIANT)c881.a ; \
	fi

	for i in *crt0.o ; \
		do \
		    if test -f $(LIB)/$(VARIANT)$$i ; \
		    then \
			    rm -f $(LIB)/OLD$(VARIANT)$$i ; \
			    cp $(LIB)/$(VARIANT)$$i $(LIB)/OLD$(VARIANT)$$i ; \
			    chmod $(PROT) $(LIB)/OLD$(VARIANT)$$i ; \
			    chmod -x $(LIB)/OLD$(VARIANT)$$i ; \
			    chgrp $(GRP) $(LIB)/OLD$(VARIANT)$$i ; \
			    chown $(OWN) $(LIB)/OLD$(VARIANT)$$i ; \
		    fi ; \
		done

clean:
	#
	# remove intermediate files except object modules and temp library
	-rm -rf lib*.contents obj*
	cd ../libc/port ;  make clean
	if m68k ; then cd ../libc/m68k ; make clean ; fi
	if m68k ; then cd m68k ; make clean ; fi
	if pdp11 ; then cd pdp11 ; make clean ; fi
	if vax ;   then cd vax ;   make clean ; fi
	if u3b ;   then cd u3b ;   make clean ; fi
	if u370 ;  then cd u370 ;  make clean ; fi

clobber: clean
	#
	# remove intermediate files
	-rm -rf *.o lib*.libc lib*.contents obj*
	cd ../libc/port ;  make clobber
	if m68k ; then cd ../libc/m68k ; make clobber ; fi
	if m68k ; then cd m68k ; make clobber ; fi
	if pdp11 ; then cd pdp11 ; make clobber ; fi
	if vax ;   then cd vax ;   make clobber ; fi
	if u3b ;   then cd u3b ;   make clobber ; fi
	if u370 ;  then cd u370 ;  make clobber ; fi
