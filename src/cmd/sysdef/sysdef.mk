#	@(#)sysdef.mk	6.1	

TARGET	=

compile all:
	-if vax; then cd dec; make -f sysdef.mk $(TARGET); fi
	-if pdp11; then cd dec; make -f sysdef.mk $(TARGET); fi
	-if u3b; then cd u3b20; make -f sysdef.mk $(TARGET); fi
	-if m68k; then cd m68k; make -f sysdef.mk $(TARGET); fi

install: 
	make -f ./sysdef.mk TARGET='install'
clean:
	make -f ./sysdef.mk TARGET='clean'
clobber:
	make -f ./sysdef.mk TARGET='clobber'
