#	@(#)makefile	7.2	
#	@(#)makefile	6.1 
#
# makefile for libc881/m68k
#
#
# The variable PROF is null by default, causing profiled object to be
# maintained.  If profiled object is not desired, the reassignment
# PROF=@# should appear in the make command line.
#

ROOT =

.SUFFIXES: .p

M4=m4 ../../../libc/m68k/m4.def
MCOUNT=../../../libc/m68k/mcount.def 
CC=cc
CFLAGS=-O -DM68881
PROF=
PROT=664
GRP=bin
OWN=bin

# a makefile exists in crt for m68k
#

OBJECTS=\
gen/atof.o        gen/ecvt.o

POBJECTS=\
gen/atof.p        gen/ecvt.p


all:
	# PROCESSOR = $(PROCESSOR)
	# DBLALIGN  = $(DBLALIGN)
	# FP        = $(FP)
#
# compile crt runtime routines, a makefile exists in the directory
#
	cd crt ; make -e $(IGN) nonprof
	$(PROF)cd crt ; make -e $(IGN) prof NONPROF=@#
#
# compile gen directory
#
	make -e $(IGN) nonprof
	$(PROF)make -e $(IGN) prof NONPROF=@#

nonprof: $(OBJECTS) gen/sysm68k.o

prof: $(POBJECTS) gen/sysm68k.p

.c.o .c.p:
	@echo $*.c:
	$(NONPROF)$(CC) $(CFLAGS) -c $*.c  && mv $(*F).o $*.o
	$(PROF)$(CC) -p $(CFLAGS) -c $*.c  && mv $(*F).o $*.p

.s.o .s.p:
	@echo $*.s:
	$(NONPROF)$(M4) -DMCOUNT=#  $*.s   > $(*F).s   && \
	$(NONPROF)$(CC) $(CFLAGS) -c $(*F).s  && mv $(*F).o $*.o
	$(PROF)$(M4) $(MCOUNT) $*.s   > $(*F).s   && $(CC) $(CFLAGS) -c $(*F).s  && mv $(*F).o $*.p
	-rm $(*F).s

gen/sysm68k.o gen/sysm68k.p:
	@echo $*.c:
	$(NONPROF)M68000= STACKCHECK=OFF $(CC) $(CFLAGS) -c $*.c  && mv $(*F).o $*.o
	$(PROF)M68000= STACKCHECK=OFF $(CC) -p $(CFLAGS) -c $*.c  && mv $(*F).o $*.p
	
archive:
	#
	# Note that "archive" is invoked with libc/object as current directory.
	#
	# figure out the correct ordering for all the archive modules except
	#	cuexit and fakcu, which must go at end of archive.
	chmod $(PROT) *.o 
	chgrp $(GRP)  *.o
	chown $(OWN)  *.o
	mv cuexit.o cuexit.x
	mv fakcu.o fakcu.x
	$(LORDER) *.o | tsort >objlist
	mv cuexit.x cuexit.o
	mv fakcu.x fakcu.o
	#
	# build the archive with the modules in correct order.
	$(AR) q ../lib.libc `cat objlist` cuexit.o fakcu.o
	$(AR) t ../lib.libc >../lib.contents
	rm -f *.o
	$(PROF)#
	$(PROF)# build the profiled library archive, first renaming the
	$(PROF)#	.p (profiled object) modules to .o
	$(PROF)for i in *.p ; do mv $$i `basename $$i .p`.o ; done
	$(PROF)chmod $(PROT) *.o
	$(PROF)chgrp $(GRP)  *.o
	$(PROF)chown $(OWN)  *.o
	$(PROF)$(AR) q ../libp.libc `cat objlist` cuexit.o fakcu.o
	$(PROF)$(STRIP) -r ../libp.libc
	$(PROF)$(AR) ts ../libp.libc >../libp.contents

clean:
	-rm -f *.[ops]

clobber: clean
	-rm -f */*.[op] */temp.s
