#ident	"@(#)enpconfig, enpload:enpdl.mk	1.0"

ROOT=
INCRT = $(ROOT)/usr/include
INSDIR = $(ROOT)/etc
INSDIR2 = $(ROOT)/usr/bin
TCP = $(ROOT)/tcpip
TCPDIR = $(ROOT)/tcpip/enp
CFLAGS = -O
LDFLAGS = -s 
INS=cpset

DATFILES = hosts hosts.equiv networks services protocols enp_param.desc
SHCMDS   = download_enp

all:	enpconfig enpload

enpconfig: enpconfig.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o enpconfig enpconfig.o -lcmcnet $(LDLIBS)

enpload: enpload.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o enpload enpload.o -lcmcnet $(LDLIBS)

enpconfig.o: enpconfig.c
	$(CC) $(CFLAGS) -I$(INCRT) -c enpconfig.c

enpload.o: enpload.c
	$(CC) $(CFLAGS) -I$(INCRT) -c enpload.c

install: all $(DATFILES) $(SHCMDS) enpcode.S
	$(INS) enpconfig $(INSDIR2) 0755 root sys
	$(INS) enpload $(INSDIR) 04755 root sys
	$(INS) hosts $(INSDIR) 0444 root sys
	$(INS) hosts.equiv $(INSDIR) 0444 root sys
	$(INS) networks $(INSDIR) 0444 root sys
	$(INS) services $(INSDIR) 0444 root sys
	$(INS) protocols $(INSDIR) 0444 root sys
	-@if [ ! -d "$(TCP)" ] ; \
	then \
	   mkdir $(TCP) $(TCPDIR) ; \
	fi;
	$(INS) download_enp $(TCPDIR) 0755 root sys
	$(INS) enp_param.desc $(TCPDIR) 0644 root sys
	$(INS) enp_param.S $(TCPDIR) 0644 root sys
	$(INS) enpcode.S $(TCPDIR) 0444 root sys
	ln $(TCPDIR)/enp_param.desc $(TCPDIR)/default.desc
	ln $(TCPDIR)/enp_param.S $(TCPDIR)/default.S

clean:
	-rm -f enpconfig.o enpload.o

clobber: clean
	rm -f enpconfig enpload
