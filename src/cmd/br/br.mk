# Makefile for making and installing the B&R system

UTILS	= cmds/common/newer cmds/common/xdate


install: $(UTILS)
	./Install /backups	# installs /backups portions


cmds/common/newer: cmds/cmds/newer.c
	$(CC) $(CFLAGS) $?  -o $@

cmds/common/xdate: cmds/cmds/xdate.c
	$(CC) $(CFLAGS) $?  -o $@

#####################################

clobber:
	rm -f $(UTILS)
