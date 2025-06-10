#
# "$Header$"
#

ROOT =
TESTDIR = .
CFLAGS = -O
LDFLAGS = -s
CC=cc
LD=ld
AS=as

all:
	@echo "\n**** igf command"
	cd igf ;	$(MAKE) -ef igf.mk
	@echo "\n**** ixf command"
	cd ixf ;	$(MAKE) -ef ixf.mk
	@echo "\n**** ipl130 bootloader"
	cd 350ipl ;	$(MAKE) -ef 350ipl.mk

install:
	@echo "\n**** igf command"
	cd igf ;	$(MAKE) -ef igf.mk	install
	@echo "\n**** ixf command"
	cd ixf ;	$(MAKE) -ef ixf.mk	install
	@echo "\n**** ipl130 bootloader"
	cd 350ipl ;	$(MAKE) -ef 350ipl.mk	install

clean:
	@echo "\n**** igf command"
	cd igf ;	$(MAKE) -ef igf.mk	clean
	@echo "\n**** ixf command"
	cd ixf ;	$(MAKE) -ef ixf.mk	clean
	@echo "\n**** ipl130 bootloader"
	cd 350ipl ;	$(MAKE) -ef 350ipl.mk	clean

clobber:
	@echo "\n**** igf command"
	cd igf ;	$(MAKE) -ef igf.mk	clobber
	@echo "\n**** ixf command"
	cd ixf ;	$(MAKE) -ef ixf.mk	clobber
	@echo "\n**** ipl130 bootloader"
	cd 350ipl ;	$(MAKE) -ef 350ipl.mk	clobber
