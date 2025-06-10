#ident	"@(#)fpack/fpack.mk	1.0"
ROOT =
INCRT = ..
LIBNAME = ../cf/lib.fpack

DASHG =
DASHO = -O
PFLAGS = $(DASHG) -I$(INCRT) -DINKERNEL 
CFLAGS = $(DASHO) $(PFLAGS)
FRC =

DFILES = \
	$(LIBNAME)(pts8023.o)

all:	pickup $(LIBNAME)

.PRECIOUS:	$(LIBNAME)

pickup:
	-@if [ "`ls *.o 2>/dev/null`" ] ; \
	then \
		echo "\tar ruv $(LIBNAME) *.o" ; \
		ar ruv $(LIBNAME) *.o ; \
		echo "\trm -f *.o" ; \
		rm -f *.o  ; \
	fi

$(LIBNAME):	$(DFILES)
	ar ruv $(LIBNAME) *.o
	rm -f *.o

.c.a:
	$(CC) $(CFLAGS) -c $<

clean:
	-rm -f *.o

clobber:
	-rm -f *.o
	-rm -f $(LIBNAME)

FRC:

#
# Header dependencies
#

$(LIBNAME)(pst8023.o): pst8023.c \
	$(INCRT)/sys/errno.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/bpp/types.h \
	$(INCRT)/sys/bpp/ce.h \
	$(INCRT)/sys/bpp/ci_config.h \
	$(INCRT)/sys/bpp/ci_const.h \
	$(INCRT)/sys/bpp/ci_types.h \
	$(INCRT)/sys/bpp/bpp.h \
	$(INCRT)/sys/fpack/ma_def.h \
	$(INCRT)/sys/fpack/ma_epb.h \
	$(INCRT)/sys/fpack/pst8023.h \
	$(FRC)
