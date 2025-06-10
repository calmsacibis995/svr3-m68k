#	@(#)error.mk	6.1	
#
# error commands makefile
#

TARGET =

all:
	if m68k; then cd m68k; make -f error.mk $(TARGET); fi
	if vax; then cd dec; make -f error.mk $(TARGET); fi
	if pdp11; then cd dec; make -f error.mk $(TARGET); fi
	if u3b; then cd u3b20; make -f error.mk $(TARGET); fi

install: 
	make -f ./error.mk TARGET='install'
clean:
	make -f ./error.mk TARGET='clean'
clobber:
	make -f ./error.mk TARGET='clobber'
