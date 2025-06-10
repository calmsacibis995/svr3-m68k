INCLUDE=/usr/include

dcon:  dcon.c $(INCLUDE)/sys/mk68564.h
	cc -I$(INCLUDE) -O -o dcon dcon.c

clobber:
	rm dcon

install: dcon
	/etc/install dcon	

