ROOT =
#INCRT = ..
INCRT = /usr/include
LIBNAME = ../cf/lib.bpp

R2 = -UR2
DASHG =
DASHO = -O
CETRACE = -DCETRACE
PFLAGS	= -I$(INCRT) -DINKERNEL
CFLAGS = $(DASHO) $(PFLAGS)
DFLAGS	= -D$(SYSTEM) $(CETRACE) $(R2)
FRC =

DFILES = \
		$(LIBNAME)(bpp_driver.o) \
		$(LIBNAME)(bppci.o) \
		$(LIBNAME)(ci_buf_mgt.o) \
		$(LIBNAME)(ci_vmexfer.o) \
		$(LIBNAME)(ci_config.o) \
		$(LIBNAME)(ci_init.o) \
		$(LIBNAME)(alloc_mem.o) \
		$(LIBNAME)(trace.o) \
		$(LIBNAME)(vme330_1.o) \
		$(LIBNAME)(vme333_1.o) \
		$(LIBNAME)(vme333_2.o) \
                $(LIBNAME)(sprobe.o)

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
	$(CC) $(CFLAGS) $(DFLAGS) -c $<

.s.a:
	$(AS) $<

clean:
	-rm -f *.o

clobber:
	-rm -f *.o
	-rm -f $(LIBNAME)

FRC:

