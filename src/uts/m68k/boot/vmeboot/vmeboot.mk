#	 vmeboot.mk %W% %Q%

INSDIR =  ???

# disk bootloader for systems with a VMEbug firmware monitor or equivalent

vmeboot: vmeboot.s ld.vmeboot
	as -n -o vmeboot.o vmeboot.s
	ld vmeboot.o ld.vmeboot -o vmeboot
	rm -f vmeboot.o

nvmeboot: nvmeboot.s ld.vmeboot
	as -n -o nvmeboot.o nvmeboot.s
	ld nvmeboot.o ld.vmeboot -o nvmeboot
	rm -f nvmeboot.o

install: vmeboot
	cp vmeboot $(INSDIR)

clean:
	rm -f *.o

clobber:	clean
	rm -f vmeboot nvmeboot
