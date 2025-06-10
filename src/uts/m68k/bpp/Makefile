##MKH###################### MAKEFILE HEADER ###########################
##
##  MAKEFILE NAME:	/u/ce/common/vmp/host/Makefile
##
##  DESCRIPTION:	This makefile builds the object modules 
##			which comprise the VME Common Interface
##
##			lib.5.2:   holds all the common interface
##				routines.
##
##  NOTES: 		The .c.o rule is provided to overcome the
##			problems of compiling source files from the
##			common/vmp/host directory from this makefile's
##			directory.
##	
##
##
##  CHANGES:     DESCRIPTION                       DATE     ENGINEER
##  ___________________________________________________________________
##
##  Initial coding.                               04/16/86    ror
##
#######################################################################


OUTPUT	=

PROCESSOR  = M68020
STACKCHECK = OFF
SYSTEM     = VME131

R2	= -DR2
#DEBUG	= -DDEBUG
CETRACE = -DCETRACE
USE 	= -DNOT_USED

CC	= cc
CFLAGS	= -O $(R2) $(DEBUG) $(USE) -I$(INCLUDE) -D$(SYSTEM) $(CETRACE) -DSYS=$(SYS)

AS	= as
ASFLAGS = -m

LD	=
LDFLAGS = 

AR	= ar
ARFLAGS = -r

LIB	= lib.5.2

RM	= /bin/rm -f
INCLUDE = include.5.2

#
#  	Source Macros
#	-------------

SOURCE = \
		bpp_driver.c \
		bppci.c \
		alloc_mem.c \
		ci_buf_mgt.c \
		ci_vmexfer.s \
		ci_config.c \
		ci_init.c \
		trace.c \
		sprobe.s \
		vme330_1.c \
		vme333_1.c \
		vme333_2.c 
#

HEADERS	      = \
		$(INCLUDE)/sys/bpp/bpp.h \
		$(INCLUDE)/sys/bpp/ce.h \
		$(INCLUDE)/sys/bpp/ci_config.h \
		$(INCLUDE)/sys/bpp/ci_const.h \
		$(INCLUDE)/sys/bpp/ci_types.h \
		$(INCLUDE)/sys/bpp/panic.h \
		$(INCLUDE)/sys/bpp/trace.h

 
#
#  	Object Macros
#	-------------
TRACEOBJ	= trace.o

OBJECT    =	\
		bpp_driver.o \
		bppci.o \
		alloc_mem.o \
		ci_buf_mgt.o \
		ci_vmexfer.o \
		ci_config.o \
		ci_init.o \
		trace.o \
		vme330_1.o \
		vme333_1.o \
		vme333_2.o \
                sprobe.o

#
#  	Major Targets 
#	=============

#------------------------------------------

bpp:	$(OBJECT)
	$(AR) $(ARFLAGS) $(LIB) $(OBJECT)

#------------------------------------------

compile:	$(OBJECT)

#------------------------------------------

v330:
	cp 330config.c ci_config.c

v333:
	cp 333config.c ci_config.c

#------------------------------------------

clean:
		$(RM) *.o *.map $(OUTPUT)


arc:		ARCHIVE

ARCHIVE:
		ls Makefile $(SOURCE) \
		| cpio -oBcv >test.ar

#------------------------------------------

bpp.flow:	$(SOURCE)
		cflow -i_ $(CFLAGS) $(SOURCE)    2>/dev/null >bpp.flow
		cflow -i_ -r $(CFLAGS) $(SOURCE) 2>/dev/null >bpp.calls

#------------------------------------------

#
#  	Minor Targets
#	-------------

bpp_driver.o \
ci_config.o  \
ci_init.o    \
ci_buf_mgt.o \
trace.o \
bppci.o: 	$(MAKEFILE) $(HEADERS)

ci_vmexfer.o \
sprobe.o: 	$(MAKEFILE)

vme330_1.o \
vme333_2.o \
vme333_1.o: 	$(MAKEFILE) \
		$(INCLUDE)/types.h \



