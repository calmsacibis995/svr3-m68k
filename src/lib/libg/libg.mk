#	@(#)libg.mk	6.1 
all:	
	@set +e; \
	if vax; \
	then \
		cd vax; $(MAKE) -f libg.mk all; \
	elif u3b; \
	then \
		cd u3b; $(MAKE) -f libg.mk all; \
	elif m68k; \
	then \
		cd m68k; $(MAKE) -e -f libg.mk all; \
	fi

install:	
	@set +e; \
	if vax; \
	then \
		cd vax; $(MAKE) -f libg.mk install; \
	elif u3b; \
	then \
		cd u3b; $(MAKE) -f libg.mk install; \
	elif m68k; \
	then \
		cd m68k; $(MAKE) -e -f libg.mk install; \
	fi

clobber:	
	@set +e; \
	if vax; \
	then \
		cd vax; $(MAKE) -f libg.mk clobber; \
	elif u3b; \
	then \
		cd u3b; $(MAKE) -f libg.mk clobber; \
	elif m68k; \
	then \
		cd m68k; $(MAKE) -e -f libg.mk clobber; \
	fi
