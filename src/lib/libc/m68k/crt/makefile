#	MC68000 C RUNTIME MAKEFILE
#
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

.SUFFIXES: .p

ROOT=
INC=$(ROOT)/usr/include
INCSYS=$(ROOT)/usr/include/sys

M4=m4 ../m4.def
AS=	as
CC=	cc
LINT=	lint
AFLAGS=	r
CFLAGS=	-O
LFLAGS=	-p
LDFLAGS=
PROF=

FPPO=	addmag.o fladd.o flcmp.o fldiv.o flmul.o flneg.o \
	flsub.o fltst.o ftol.o itof.o ltof.o \
	submag.o uitof.o ultof.o \
	dbadd.o dbcmp.o dbdiv.o dbmul.o dbneg.o dbtofl.o \
	dbtol.o dbtst.o fltodb.o ltodb.o fpdata.o table.o

FPPP=	addmag.p fladd.p flcmp.p fldiv.p flmul.p flneg.p \
	flsub.p fltst.p ftol.p itof.p ltof.p \
	submag.p uitof.p ultof.p \
	dbadd.p dbcmp.p dbdiv.p dbmul.p dbneg.p dbtofl.p \
	dbtol.p dbtst.p fltodb.p ltodb.p fpdata.p table.p

FPPC=	addmag.c fladd.c flcmp.c fldiv.c flmul.c flneg.c \
	flsub.c fltst.c ftol.c itof.c ltof.c \
	submag.c uitof.c ultof.c

OTHERO=	chrtab.o ldiv.o lmul.o cerror.o sigcode.o spgrow.o mcount.o
OTHERP=	chrtab.p ldiv.p lmul.p cerror.p sigcode.p spgrow.p mcount.p
OTHERC= chrtab.c

OFILES=		$(FPPO) $(OTHERO)
PFILES=		$(FPPP) $(OTHERP)
CFILES=		$(FPPC) $(OTHERC)


nonprof:	$(OFILES)

prof:		$(PFILES)

$(FPPO):	$(INCSYS)/fp.h

#-------------------------

.c.o .c.p:
# This procedure is overkill, but harmless, for the non-FPP routines
	@echo $*.c:
	$(PROF)if test -f $*.o ; then mv $*.o tmp.o ; fi ; \
		$(CC) -p $(CFLAGS) -S $<
	$(NONPROF)$(CC) $(CFLAGS) -S $<
	sed -f massage.sed <$*.s >temp
	mv temp $*.s
	$(PROF)$(CC) -p $(CFLAGS) -c $*.s && mv $*.o $*.p ; \
		if test -f tmp.o ; then mv tmp.o $*.o ; fi
	$(NONPROF)$(CC) $(CFLAGS) -c $*.s
	rm $*.s

#-------------------------

.s.o .s.p:
	@echo $*.s:
	$(PROF)if test -f $*.o ; then mv $*.o tmp.o ; fi ; \
		$(M4) ../mcount.def  $*.s   > temp.s   && \
		$(CC) $(CFLAGS) -c temp.s  && mv temp.o $*.p ; \
		if test -f tmp.o ; then mv tmp.o $*.o ; fi
	$(NONPROF)$(M4) -DMCOUNT=#  $*.s   > temp.s   && \
	$(NONPROF)$(CC) $(CFLAGS) -c temp.s && mv temp.o $*.o

#--------------------------

clean:
	-rm -f $(OFILES) $(PFILES) temp temp.o temp.s

#--------------------------

clobber:	clean

#--------------------------

lint:	$(CFILES) $(INCSYS)/fp.h
	$(LINT) $(LFLAGS) $(CFILES) >lint.out
