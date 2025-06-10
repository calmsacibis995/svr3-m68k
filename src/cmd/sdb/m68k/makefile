#	@(#)makefile	6.8	

FPS=M68881
FPS=NOFPU
DEFINES= -DFLEXNAMES -D$(PROCESSOR)  -DSDB -D$(FPS)
CFLAGS=	-O -DDEBUG $(DEFINES)
CFLAGS= -DDEBUG=2  $(DEFINES)
CFLAGS=	-O
LDFLAGS =
LDFLAGS = -s

CC_CMD = $(CC) $(CFLAGS) $(DEFINES) $(LDFLAGS)

LIBFLAGS = 
ROOT =
UBIN = $(ROOT)/usr/bin
TESTDIR = ..
INSUB = /etc/install -f $(UBIN)
INSDIR =
SRC = $(ROOT)/usr/src/cmd
DISDIR = $(SRC)/sgs/dis/m68k
DISDIRP = $(DISDIR)/$(PROCESSOR)
DISINC = -I$(DISDIR) -I$(DISDIRP) -I$(SRC)/sgs/inc/m68k

DIS.H=	$(DISDIR)/dis.h	$(DISDIRP)/names.h
DIS.C=	$(DISDIRP)/text_dis.c	$(DISDIRP)/names.c	$(DISDIRP)/881_gen.c \
	$(DISDIRP)/cp_gen.c	$(DISDIRP)/extended.c	dis_utils.c
DIS.O=	text_dis.o names.o 881_gen.o cp_gen.o extended.o dis_utils.o

OBJS=	main.o re.o fio.o bio.o decode.o docomm.o sub.o  \
	access.o pcs.o runpcs.o setup.o message.o \
	symt.o display.o prvar.o xeq.o version.o \
	opset.o optab.o machdep.o udef.o \
	debug.o

COM= ../com
SOURCE=	$(COM)/main.c $(COM)/re.c $(COM)/fio.c $(COM)/bio.c $(COM)/decode.c  \
	$(COM)/docomm.c $(COM)/sub.c $(COM)/access.c $(COM)/pcs.c  \
	$(COM)/runpcs.c $(COM)/setup.c $(COM)/message.c  \
	$(COM)/symt.c $(COM)/display.c $(COM)/prvar.c $(COM)/xeq.c  \
	$(COM)/version.c $(COM)/opset.c $(COM)/optab.c  \
	$(COM)/machdep.c $(COM)/udef.c $(COM)/debug.c 

CSOURCE= $(COM)/machdep.h $(COM)/access.c $(COM)/machdep.c $(COM)/opset.c \
	$(COM)/optab.c $(COM)/runpcs.c $(COM)/setup.c $(COM)/xeq.c

compile all: $(TESTDIR)/sdb

$(TESTDIR)/sdb:	$(OBJS) $(DIS.O)
	cc $(LDFLAGS) -o $(TESTDIR)/sdb $(OBJS) $(DIS.O) $(LIBFLAGS)


COFF.H= $(COM)/coff.h $(COM)/sgs.h 
MACHDEP.H= $(COM)/machdep.h 
MODE.H= $(COM)/mode.h
HEAD.H= $(COM)/head.h $(COM)/bio.h $(MODE.H) $(MACHDEP.H) 


decode.o:	$(HEAD.H) $(COM)/decode.c
		$(CC_CMD) -c $(CFLAGS) $(COM)/decode.c

docomm.o:	$(HEAD.H) $(COFF.H) $(COM)/docomm.c
		$(CC_CMD) -c $(CFLAGS) $(COM)/docomm.c

main.o:		$(HEAD.H) $(COM)/main.c
		$(CC_CMD) -c $(CFLAGS) $(COM)/main.c

sub.o:		$(HEAD.H) $(COFF.H) $(COM)/sub.c
		$(CC_CMD) -c $(CFLAGS) $(COM)/sub.c

re.o:		$(HEAD.H) $(COM)/re.c
		$(CC_CMD) -c $(CFLAGS) $(COM)/re.c

fio.o:		$(HEAD.H) $(COM)/fio.c
		$(CC_CMD) -c $(CFLAGS) $(COM)/fio.c

bio.o:		$(COM)/bio.h $(COM)/bio.c
		$(CC_CMD) -c $(CFLAGS) $(COM)/bio.c

access.o:	$(HEAD.H) $(COM)/access.c
		$(CC_CMD) -c $(CFLAGS) $(COM)/access.c

pcs.o:		$(HEAD.H) $(COM)/pcs.c
		$(CC_CMD) -c $(CFLAGS) $(COM)/pcs.c

runpcs.o:	$(HEAD.H) $(COM)/runpcs.c
		$(CC_CMD) -c $(CFLAGS) $(COM)/runpcs.c

xeq.o:		$(HEAD.H) $(COFF.H) $(COM)/xeq.c
		$(CC_CMD) -c $(CFLAGS) $(COM)/xeq.c

setup.o:	$(HEAD.H) $(COFF.H) $(COM)/setup.c
		$(CC_CMD) -c $(CFLAGS) $(COM)/setup.c

message.o:	$(MODE.H) $(COM)/message.c
	$(CC_CMD) $(CFLAGS) -S $(COM)/message.c
	ed - <$(COM)/:rofix message.s
	as -o message.o message.s
	rm message.s

symt.o:		$(HEAD.H) $(COFF.H) $(COM)/symt.c
		$(CC_CMD) -c $(COM)/symt.c

display.o:	$(HEAD.H) $(COFF.H)  $(COM)/display.c
		$(CC_CMD) -c $(COM)/display.c

prvar.o:	$(HEAD.H) $(COFF.H) $(COM)/prvar.c
		$(CC_CMD) -c $(CFLAGS) $(COM)/prvar.c

version.o:	$(COM)/version.c
		$(CC_CMD) -c $(CFLAGS) $(COM)/version.c

opset.o:	$(HEAD.H) $(COM)/opset.c
		$(CC_CMD) -c $(CFLAGS) $(COM)/opset.c

optab.o:	$(MODE.H) $(MACHDEP.H)  $(COM)/optab.c
	$(CC_CMD) $(CFLAGS) -S $(COM)/optab.c
	ed - <$(COM)/:rofix optab.s
	as -o optab.o optab.s
	rm optab.s
machdep.o:	$(HEAD.H) $(COFF.H) $(COM)/machdep.c
		$(CC_CMD) -c $(CFLAGS) $(COM)/machdep.c

udef.o:		$(HEAD.H) $(COFF.H) $(COM)/udef.c
		$(CC_CMD) -c $(CFLAGS) $(COM)/udef.c

#
#			Build files for disassembler.
#			source is from the utility 'dis'.
#
disobjs:	$(DIS.O) $(DIS.H)
text_dis.o: $(DISDIRP)/text_dis.c $(DIS.H)
	$(CC_CMD) -c $(CFLAGS) $(DISINC) $(DISDIRP)/text_dis.c

names.o: $(DISDIRP)/names.c $(DIS.H)
	$(CC_CMD) -c $(CFLAGS) $(DISINC) $(DISDIRP)/names.c

extended.o: $(DISDIRP)/extended.c $(DIS.H)
	$(CC_CMD) -c $(CFLAGS) $(DISINC) $(DISDIRP)/extended.c

881_gen.o: $(DISDIRP)/881_gen.c $(DIS.H)
	$(CC_CMD) -c $(CFLAGS) $(DISINC) $(DISDIRP)/881_gen.c

cp_gen.o: $(DISDIRP)/cp_gen.c $(DIS.H)
	$(CC_CMD) -c $(CFLAGS) $(DISINC) $(DISDIRP)/cp_gen.c

dis_utils.o: dis_utils.c $(DIS.H)
	$(CC_CMD) -c $(CFLAGS) $(DISINC) dis_utils.c
#
#			End of disassembler related files
#

debug.o:	$(COM)/head.h $(COFF.H) $(COM)/debug.c
		$(CC_CMD) -c $(CFLAGS) $(COM)/debug.c

install:	all
		$(INSUB) $(TESTDIR)/sdb 

clean :
		-rm -f $(OBJS) $(DIS.O)

clobber: clean
		-rm -f $(TESTDIR)/sdb

