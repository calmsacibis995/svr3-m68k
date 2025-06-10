#	@(#)libg.mk	6.1 
OWN=	bin		#file owner
GRP=	bin		#file group
PROT=	664		#protections

ROOT = 
LIB= $(ROOT)/usr/lib

TESTDIR=.
all:	libg.a

libg.a: dbxxx.s
	as -o $(TESTDIR)/libg.a dbxxx.s

clean:
	rm -f *.o

clobber: clean
	rm -f libg.a

move:
	#
	# move the library or libraries into the correct directory

	rm -f $(LIB)/libg.a
	cp libg.a $(LIB)/libg.a
	chmod $(PROT) $(LIB)/libg.a
	chmod -x $(LIB)/libg.a	# not executable
	chgrp $(GRP) $(LIB)/libg.a
	chown $(OWN) $(LIB)/libg.a

install: all save move

#--------------------------

uninstall:	$(LIB)/OLDlibg.a
		
		mv -f $(LIB)/OLDlibg.a $(LIB)/libg.a

#--------------------------

save:		
	if test -f $(LIB)/libg.a ; \
	then \
		rm -f $(LIB)/OLDlibg.a ; \
		cp $(LIB)/libg.a $(LIB)/OLDlibg.a ; \
		chmod $(PROT) $(LIB)/OLDlibg.a ; \
		chmod -x $(LIB)/OLDlibg.a ; \
		chgrp $(GRP) $(LIB)/OLDlibg.a ; \
		chown $(OWN) $(LIB)/OLDlibg.a ; \
	fi
