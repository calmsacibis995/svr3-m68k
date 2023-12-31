#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)crash:crash.mk	1.0"
STRIP = 
DBO = -DDBO
MAKE = make "AS=$(AS)" "CC=$(CC)" "LD=$(LD)"

TESTDIR = .
ROOT =
INSDIR = $(ROOT)/etc
INS = install -n $(INSDIR)
LDFLAGS = -s
INCRT=$(ROOT)/usr/include
COMFLAGS = -I$(INCRT) -I. -Uvax -Uu3b -Updp11 -Uu3b5 -Uu3b2 $(DBO)
CFLAGS= $(COMFLAGS) 
FRC =

# dis.o removed util it works - wjc 01/23/87 - at that time, 
# also remove #ifdef DIS in main.c

OFILES= base.o \
	buf.o \
	callout.o \
	fs.o \
	init.o \
	inode.o \
	lck.o \
	main.o \
	major.o \
	map.o \
	mount.o \
	nvram.o \
	page.o \
	proc.o \
	rfs.o \
	search.o \
	size.o \
	sizenet.o \
	stream.o \
	symtab.o \
	tty.o \
	u.o \
	util.o \
	var.o \
	vtop.o 

CFILES= base.c \
	buf.c \
	callout.c \
	fs.c \
	init.c \
	inode.c \
	lck.c \
	main.c \
	major.c \
	map.c \
	mount.c \
	nvram.c \
	page.c \
	proc.c \
	rfs.c \
	search.c \
	size.c \
	sizenet.c \
	stream.c \
	symtab.c \
	tty.c \
	u.c \
	util.c \
	var.c \
	vtop.c 


all: crash

crash:	$(OFILES) vuifile
	$(CC) $(CFLAGS) $(FFLAG) $(LDFLAGS) -o $(TESTDIR)/crash vuifile $(OFILES) $(LDLIBS)

install: ins_crash

ins_crash: crash
	$(INS) $(TESTDIR)/crash $(INSDIR)
	$(CH) chmod 755 $(INSDIR)/crash

clean:
	-rm -f *.o

clobber: clean
	-rm -f $(TESTDIR)/crash

xref: $(CFILES) $(HFILES) vuifile
	cxref -c $(COMFLAGS) $(CFILES) vuifile | pr -h crash.cxref | opr

lint: $(CFILES) $(HFILES) vuifile
	lint $(COMFLAGS) -uh $(CFILES) vuifile

prall:
	pr -n $(CFILES) | opr
	pr -n $(HFILES) | opr

FRC:
