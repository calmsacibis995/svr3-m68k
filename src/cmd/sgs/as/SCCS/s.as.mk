h19994
s 00070/00000/00000
d D 1.1 86/08/04 09:41:16 fnf 1 0
c Start with baseline 5.2 68000 assembler.
e
u
U
t
T
I 1
#		Copyright (c) 1985 AT&T		#
#		All Rights Reserved		#
#	@(#)as.mk	6.1	

#	as.mk as makefile

ROOT=
INSDIR = $(ROOT)/bin
CURDIR = ..
#
# CFLAGS should also be passed down, but that won't be done until
# the makefiles are standardized
#

as all:
	-if vax; \
	then \
		cd vax; \
		$(MAKE) -$(MAKEFLAGS) CC="$(CC)" INSDIR=$(CURDIR); \
		cd ../lj_vax; \
		$(MAKE) -$(MAKEFLAGS) CC="$(CC)" INSDIR=$(CURDIR); \
	elif m68k; \
	then \
		cd m68k; \
		$(MAKE) -$(MAKEFLAGS) CC="$(CC)" INSDIR=$(CURDIR); \
	elif pdp11; \
	then \
		cd pdp11; \
		$(MAKE) -$(MAKEFLAGS) CC="$(CC)" INSDIR=$(CURDIR); \
	else \
		echo 'Cannot make as command: unknown target procesor.'; \
	fi



clean:
	-if vax; \
	then \
		cd vax; \
		$(MAKE) clean; \
		cd ../lj_vax; \
		$(MAKE) clean; \
	elif m68k; \
	then \
		cd m68k; \
		$(MAKE) clean; \
	elif pdp11; \
	then \
		cd pdp11; \
		$(MAKE) clean; \
	fi


install: as
	-if vax; \
	then \
		cp as $(INSDIR)/as; \
		cp ljas $(INSDIR)/ljas; \
	elif m68k; \
	then \
		cp m68k/as $(INSDIR)/as; \
		rm -f $(INSDIR)/ljas; \
		ln $(INSDIR)/as $(INSDIR)/ljas; \
	elif pdp11; \
	then \
		cp as2 /lib/as2; \
	fi

clobber: clean
	-rm -f ljas as as2
E 1
