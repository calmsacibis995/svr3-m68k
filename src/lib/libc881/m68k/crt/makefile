#	MC68000 C RUNTIME MAKEFILE
#
#	@(#)makefile	6.1	
#	@(#)makefile	6.3	
#
# The variable PROF is null by default, causing both the standard C library
# and a profiled library to be maintained.  If profiled object is not 
# desired, the reassignment PROF=@# should appear in the make command line.
#
# The variable IGN may be set to -i by the assignment IGN=-i in order to
# allow a make to complete even if there are compile errors in individual
# modules.
#
#

ROOT=
INC=$(ROOT)/usr/include
INCSYS=$(ROOT)/usr/include/sys
LIBCM68K=../../../libc/m68k
LIBCM68KCRT=$(LIBCM68K)/crt

M4=m4 $(LIBCM68K)/m4.def
MASSAGE=$(LIBCM68KCRT)/massage.sed
AS=	as
CC=	cc
LINT=	lint
AFLAGS=	r
CFLAGS=	-O -I$(LIBCM68KCRT)
LFLAGS=	-p
LDFLAGS=
PROF=

FPPO=	         fladd.o         fldiv.o flmul.o         \
	                ftol.o itof.o \
	         uitof.o \
	dbadd.o dbcmp.o dbdiv.o dbmul.o dbtofl.o \
	access81.o \
	dbtol.o fltodb.o ltodb.o

FPPP=	         fladd.p         fldiv.p flmul.p         \
	                ftol.p itof.p \
	         uitof.p \
	dbadd.p dbcmp.p dbdiv.p dbmul.p dbtofl.p \
	access81.p \
	dbtol.p fltodb.p ltodb.p         

OFILES=		$(FPPO)
PFILES=		$(FPPP)


nonprof:	$(OFILES)

prof:		$(PFILES)

$(FPPO):

#-------------------------

.c.o .c.p:
# This procedure is overkill, but harmless, for the non-FPP routines
	@echo $*.c:
	$(PROF)if test -f $*.o ; then mv $*.o tmp.o ; fi ; \
		$(CC) -p $(CFLAGS) -S $<
	$(NONPROF)$(CC) $(CFLAGS) -S $<
	sed -f $(MASSAGE) <$*.s >temp
	mv temp $*.s
	$(PROF)$(CC) -p $(CFLAGS) -c $*.s && mv $*.o $*.p ; \
		if test -f tmp.o ; then mv tmp.o $*.o ; fi
	$(NONPROF)$(CC) $(CFLAGS) -c $*.s
	rm $*.s

#-------------------------

.s.o .s.p:
	@echo $*.s:
	$(PROF)if test -f $*.o ; then mv $*.o tmp.o ; fi ; \
		$(M4) $(LIBCM68K)/mcount.def  $*.s   > temp.s   && \
		$(CC) $(CFLAGS) -c temp.s  && mv temp.o $*.p ; \
		if test -f tmp.o ; then mv tmp.o $*.o ; fi
	$(NONPROF)$(M4) -DMCOUNT=#  $*.s   > temp.s   && \
	$(NONPROF)$(CC) $(CFLAGS) -c temp.s && mv temp.o $*.o

#--------------------------

clean:
	-rm -f $(OFILES) $(PFILES) temp temp.o temp.s

#--------------------------

clobber:	clean

