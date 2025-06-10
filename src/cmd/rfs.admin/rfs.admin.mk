#ident	"@(#)rfs.admin:rfs.admin.mk	2.3"
#
#	This makefile makes appropriate directories
#	and copies the simple admin shells into
#	the rfsmgmt directory
#

OWNER=bin
GRP=bin
MGMT=$(ROOT)/usr/admin/menu/packagemgmt/rfsmgmt
TCPIP=$(MGMT)/tcpip


install:
	-mkdir $(ROOT)/usr
	-mkdir $(ROOT)/usr/admin
	-mkdir $(ROOT)/usr/admin/menu
	-mkdir $(ROOT)/usr/admin/menu/packagemgmt
	-mkdir $(MGMT)
	cp DESC $(MGMT)/DESC
	-for i in * ;							\
	do								\
		if [ -d $$i ] ;						\
		then							\
			mkdir $(MGMT)/$$i ;				\
			cp $$i/* $(MGMT)/$$i ;				\
			chgrp $(GRP) $(MGMT)/$$i/* ;			\
			chmod 555 $(MGMT)/$$i/* ;			\
			chown $(OWNER) $(MGMT)/$$i/* ;			\
			chmod 755 $(MGMT)/$$i ;				\
			chgrp $(GRP) $(MGMT)/$$i ;			\
			chown $(OWNER) $(MGMT)/$$i ;			\
		fi ;							\
	done
	$(CH)-chgrp $(GRP) $(MGMT)/*
	$(CH)-chmod 555 $(MGMT)/*
	$(CH)-chown $(OWNER) $(MGMT)/*
	$(CH)-chmod 755 $(MGMT)
	$(CH)-chgrp $(GRP) $(MGMT)
	$(CH)-chown $(OWNER) $(MGMT)


clobber:
