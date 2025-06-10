#
#	make			builds a kernel
#

#ident	"@(#)kern-port:sysV68.mk	10.11"

INCRT=/usr/include
LIBDIR=../cf

NAME = sysV68
SYS =

DASHG =
DASHO = -O
FRC =

KNOWNLIBS = lib.ml lib.os lib.io lib.s5 lib.bpp lib.fpack lib.stub \
	lib.streams lib.du lib.fs lib.tcpip

LIBS = lib.ml lib.os lib.io lib.s5 lib.bpp lib.fpack lib.stub
CFLAGS= $(DASHO) $(DASHG) -I$(INCRT) -DINKERNEL -DREM 
XCFLAGS=

NSELIBS = lib.ml lib.os lib.io lib.s5 lib.bpp lib.fpack \
	lib.streams lib.du lib.fs lib.tcpip

NSEXCFLAGS= -DDU -DSTREAMS

MAKEARGS= "MAKE=$(MAKE)" "FRC=$(FRC)" "DASHG=$(DASHG)" "DASHO=$(DASHO)" \
	 "INCRT=$(INCRT)" "LIBDIR=$(LIBDIR)" "SYS=$(SYS)" "CFLAGS=$(CFLAGS)"

all:
	-@if [ -f Makelock ] ;						\
	then								\
	    echo Make already started by `cat Makelock` ;		\
	    exit 1 ;							\
	fi ;								\
									\
	trap 'rm -f Makelock'  0 1 2 3 15 ;				\
									\
	(echo "$$LOGNAME on \c"; date) >Makelock ;			\
									\
	$(MAKE) kernel ;						\
									\
	rm -f Makelock

kernel:	master dfile libs
	-@if [ "`grep '^du[ 	]' dfile`" != "" ] ;			\
	then								\
		SHLIBS="$(NSELIBS)" ;					\
		SHXCFLAGS="$(NSEXCFLAGS)" ;				\
	else								\
		SHLIBS="$(LIBS)" ;					\
		SHXCFLAGS= ;						\
	fi ;								\
	for i in lib.* ;						\
	do								\
		for j in $(KNOWNLIBS) ;					\
		do							\
			if [ "$$i" = "$$j" ] ;				\
			then						\
				continue 2 ;				\
			fi ;						\
		done ;							\
		SHLIBS="$$SHLIBS $$i" ;					\
	done ;								\
	: search libraries twice to resolve interdependencies ;		\
	SHLIBS="$$SHLIBS $$SHLIBS" ;					\
	rm -f conf.c conf.o config.h m68kvec.* low.* ;			\
	$(MAKE) $(NAME)$(SYS) $(MAKEARGS) LIBS="$$SHLIBS" XCFLAGS="$$SHXCFLAGS"

$(NAME)$(SYS): $(LIBS) ld.VME131 m68kvec.o low.o conf.o
	-rm -f $(NAME)$(SYS) $(NAME)$(SYS).nm
	$(LD) -o $(NAME)$(SYS) ld.VME131 m68kvec.o low.o conf.o $(LIBS)
	-@if [ -x ../kdb/namelist ] ;					\
	then								\
  	  echo "  ../kdb/namelist -L $(NAME)$(SYS) > $(NAME)$(SYS).nm";	\
		../kdb/namelist -L $(NAME)$(SYS) > $(NAME)$(SYS).nm ;	\
	fi ;								\
	if [ -f lib.kdb -a -x ../kdb/kdbnm ] ;				\
	then								\
		echo "	../kdb/kdbnm -q $(NAME)$(SYS)" ;		\
		../kdb/kdbnm -q $(NAME)$(SYS) ;				\
	fi ;								\
	if [ -x $(NAME)$(SYS) ] ;					\
	then								\
		echo $(NAME)$(SYS) made. ;				\
	fi

libs:FRC
	-@cd .. ;							\
	for i in * ;							\
	do								\
		if [ $$i != cf -a -d $$i -a -f $$i/$$i.mk ] ;		\
		then							\
			cd $$i ;					\
			echo "	"$(MAKE) -f $$i.mk $(MAKEARGS) ;	\
			$(MAKE) -f $$i.mk $(MAKEARGS) ;			\
			cd .. ;						\
		fi ;							\
	done
			
m68kvec.s low.s config.h conf.c:	master dfile
	./config -m master -v m68kvec.s -l low.s dfile

m68kvec.o:	m68kvec.s
	as -o m68kvec.o m68kvec.s

low.o:	low.s
	as -o low.o low.s

conf.o:	conf.c config.h $(INCRT)/sys/conf.h $(INCRT)/sys/param.h \
		$(INCRT)/sys/types.h $(INCRT)/sys/sysmacros.h \
		$(INCRT)/sys/space.h $(INCRT)/sys/io.h
	cc -c $(CFLAGS) $(XCFLAGS) conf.c

clean:
	-@cd .. ;							\
	for i in * ;							\
	do								\
		if [ $$i != cf -a -d $$i -a -f $$i/$$i.mk ] ;		\
		then							\
			cd $$i ;					\
			echo "	"$(MAKE) -f $$i.mk clean $(MAKEARGS) ;	\
			$(MAKE) -f $$i.mk clean $(MAKEARGS) ;		\
			cd .. ;						\
		fi ;							\
	done
	-rm -f m68kvec.s m68kvec.o low.s low.o conf.c config.h conf.o

clobber:	clean
	-@cd .. ;							\
	for i in * ;							\
	do								\
		if [ $$i != cf -a -d $$i -a -f $$i/$$i.mk ] ;		\
		then							\
			cd $$i ;					\
			echo "	"$(MAKE) -f $$i.mk clobber $(MAKEARGS) ;\
			$(MAKE) -f $$i.mk clobber $(MAKEARGS) ;	\
			cd .. ;						\
		fi ;							\
	done
	-rm -f $(NAME)$(SYS) $(NAME)$(SYS).nm

FRC:
