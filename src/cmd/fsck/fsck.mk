#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)fsck:fsck.mk	1.8"
ROOT =
TESTDIR = .
INSDIR = $(ROOT)/etc
INCRT = $(ROOT)/usr/include
INS = :
CFLAGS = -O -s
FRC =

all: fsck fsck1b fsck16b
fsck16b::\
	$(INCRT)/stdio.h\
	$(INCRT)/ctype.h\
	$(INCRT)/sys/fs/s5param.h\
	$(INCRT)/signal.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/sys/fs/s5filsys.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/fs/s5fblk.h\
	$(INCRT)/sys/ino.h\
	$(INCRT)/sys/fs/s5inode.h\
	$(INCRT)/sys/inode.h\
	$(INCRT)/sys/stat.h\
	fsck.c\
	$(FRC)
	$(CC) -I. -I$(INCRT) $(CFLAGS) -DFsTYPE=5 -o $(TESTDIR)/fsck16b fsck.c $(LDLIBS)

fsck16b::
	-if pdp11 ; then ln $(TESTDIR)/fsck16b $(TESTDIR)/fsck ; \
	$(INS) -n $(INSDIR) $(TESTDIR)/fsck ; \
	else $(INS) -n $(INSDIR) $(TESTDIR)/fsck16b ; fi

fsck::\
	$(INCRT)/stdio.h\
	$(INCRT)/ctype.h\
	$(INCRT)/sys/param.h\
	$(INCRT)/signal.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/sys/filsys.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/fs/s5fblk.h\
	$(INCRT)/sys/ino.h\
	$(INCRT)/sys/fs/s5inode.h\
	$(INCRT)/sys/inode.h\
	$(INCRT)/sys/stat.h\
	fsck.c\
	$(FRC)
	$(CC) -I$(INCRT) $(CFLAGS) -DFsTYPE=2 -o $(TESTDIR)/fsck fsck.c $(LDLIBS)

fsck::
	$(INS) -n $(INSDIR) $(TESTDIR)/fsck

fsck1b::\
	$(INCRT)/stdio.h\
	$(INCRT)/ctype.h\
	$(INCRT)/sys/param.h\
	$(INCRT)/signal.h\
	$(INCRT)/sys/types.h\
	$(INCRT)/sys/sysmacros.h\
	$(INCRT)/sys/filsys.h\
	$(INCRT)/sys/dir.h\
	$(INCRT)/sys/fs/s5fblk.h\
	$(INCRT)/sys/ino.h\
	$(INCRT)/sys/fs/s5inode.h\
	$(INCRT)/sys/inode.h\
	$(INCRT)/sys/stat.h\
	fsck.c\
	$(FRC)
	$(CC) -I$(INCRT) $(CFLAGS) -DFsTYPE=1 -o $(TESTDIR)/fsck1b fsck.c $(LDLIBS)

fsck1b::
	-if pdp11 ; then ln $(TESTDIR)/fsck1b $(TESTDIR)/fsck ; \
	$(INS) -n $(INSDIR) $(TESTDIR)/fsck ; \
	else $(INS) -n $(INSDIR) $(TESTDIR)/fsck1b ; fi

FRC :

install:
	-if pdp11 ; then make -f fsck.mk fsck1b INS=install\
		TESTDIR="$(TESTDIR)" INSDIR="$(INSDIR)"\
		INCRT="$(INCRT)" CFLAGS="$(CFLAGS)" ; \
	else make -f fsck.mk all INS=install FRC=$(FRC)\
		TESTDIR="$(TESTDIR)" INSDIR="$(INSDIR)"\
		INCRT="$(INCRT)" CFLAGS="$(CFLAGS)" ; fi

clean:
	rm -f *.o

clobber : clean
	rm -f $(TESTDIR)/fsck $(TESTDIR)/fsck1b $(TESTDIR)/fsck16b
