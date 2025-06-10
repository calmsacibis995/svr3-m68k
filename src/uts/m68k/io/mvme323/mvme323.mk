# DESCRIPTION: Create and Install the MVME323 ESDI Disk Driver

ROOT =
INCRT = ../..
LIBNAME = ../../cf/lib.io

DASHG =
DASHO = -O
PFLAGS = -I$(INCRT) -DINKERNEL 
CFLAGS = $(DASHO) $(PFLAGS)
FRC =

INCLUDES= \
	$(INCRT)/sys/param.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/signal.h\
	$(INCRT)/sys/user.h\
	$(INCRT)/sys/errno.h\
	$(INCRT)/sys/buf.h\
	$(INCRT)/sys/iobuf.h\
	$(INCRT)/sys/systm.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/sys/elog.h\
	$(INCRT)/sys/open.h\
	$(INCRT)/sys/dk.h\
	$(INCRT)/sys/cmn_err.h\
       	$(INCRT)/sys/mvme323.h

SOURCES	= \
		header.c\
                attach.c \
                buf_access.c \
                close.c \
                commands.c \
                displays.c \
                execute.c \
                go.c \
                init.c \
                interrupt.c \
                ioctl.c \
                iopb_access.c \
                logical_blk.c \
                open.c \
		print.c \
                queue_access.c \
                read.c \
                reg_access.c \
                strategy.c \
                write.c


DFILES = \
	$(LIBNAME)(mvme323.o) 

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
	$(CC) -I$(INCRT) -c $<

clean:
	-@for i in * ; \
	do \
		if [ -d $$i -a -f $$i/$$i.mk ] ; then \
			cd $$i; \
			echo "	$(MAKE) -f $$i clean"; \
			$(MAKE) -f $$i.mk clean; \
			cd ..; \
		fi ; \
	done
	-rm -f *.o

clobber:
	-@for i in * ; \
	do \
		if [ -d $$i -a -f $$i/$$i.mk ] ; then \
			cd $$i; \
			echo "	$(MAKE) -f $$i clobber"; \
			$(MAKE) -f $$i.mk clobber; \
			cd ..; \
		fi ; \
	done
	-rm -f *.o
	-rm -f $(LIBNAME)

FRC:

#
# Header dependencies
#

$(LIBNAME)(mvme323.o):	$(INCLUDES) $(SOURCES)
	cat $(SOURCES) > mvme323.c
	$(CC) -I$(INCRT) -c  mvme323.c
#	rm -f mvme323.c
FRC:
