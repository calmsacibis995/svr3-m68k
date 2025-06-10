#	@(#)dis.mk	6.1	
#	dis.mk - dis makefile

OL =
TESTDIR = .
BIN = $(OL)/bin
INSB = /etc/install -f $(BIN)

INSDIR = $(OL)/bin
INS = /etc/install -f $(INSDIR)

compile all : dis

dis:
	-if m68k; \
	then \
		 cd m68k; \
		 $(MAKE) TESTDIR=..; \
	fi

install: all
	$(INSB) $(TESTDIR)/dis

clean:
	-if m68k; \
	then \
		cd m68k; \
		$(MAKE) clean; \
	fi

clobber:
	-if m68k; \
	then \
		rm -f $(TESTDIR)/dis; \
	fi

