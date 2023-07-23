#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)adm:.adm.mk	1.18.1.2"

ROOT =
LIB = $(ROOT)/usr/lib
CRONTABS = $(ROOT)/usr/spool/cron/crontabs
LIBCRON = $(LIB)/cron
INSDIR = $(ROOT)/etc
#(mat)TOUCH=/bin/touch
TOUCH=:
LOCBIN = /local/bin

CRON_ENT= adm root sys sysadm

CRON_LIB= .proto at.allow cron.allow queuedefs

ETC_SCRIPTS= bcheckrc brc checkall checklist filesave gettydefs \
	group ioctl.syscon master motd passwd powerfail \
	rc shutdown system system.mtc11 system.mtc12 \
	system.un32 tapesave diskinfo tm220.fkey termsupport \
	termcap perms fmtflp FT45DL mklost+found

all:	etc_scripts crontab cronlib .profile

crontab: $(CRON_ENT)

cronlib: $(CRON_LIB)

etc_scripts: $(ETC_SCRIPTS)

clean:

clobber: clean

install:
	make -f .adm.mk $(ARGS)

adm::
	cp adm $(CRONTABS)/adm
	$(CH)chmod 644 $(CRONTABS)/adm
	$(CH)chgrp sys $(CRONTABS)/adm
	$(TOUCH) 0101000070 $(CRONTABS)/adm
	$(CH)chown root $(CRONTABS)/adm

root::
	cp root $(CRONTABS)/root
	$(CH)chmod 644 $(CRONTABS)/root
	$(CH)chgrp sys $(CRONTABS)/root
	$(TOUCH) 0101000070 $(CRONTABS)/root
	$(CH)chown root $(CRONTABS)/root

sys::
	cp sys $(CRONTABS)/sys
	$(CH)chmod 644 $(CRONTABS)/sys
	$(CH)chgrp sys $(CRONTABS)/sys
	$(TOUCH) 0101000070 $(CRONTABS)/sys
	$(CH)chown root $(CRONTABS)/sys

sysadm::
	cp sysadm $(CRONTABS)/sysadm
	$(CH)chmod 644 $(CRONTABS)/sysadm
	$(CH)chgrp sys $(CRONTABS)/sysadm
	$(TOUCH) 0101000070 $(CRONTABS)/sysadm
	$(CH)chown root $(CRONTABS)/sysadm

.proto::
	cp .proto $(LIBCRON)/.proto
	$(CH)chmod 744 $(LIBCRON)/.proto
	$(CH)chgrp sys $(LIBCRON)/.proto
	$(TOUCH) 0101000070 $(LIBCRON)/.proto
	$(CH)chown root $(LIBCRON)/.proto

at.allow::
	cp at.allow $(LIBCRON)/at.allow
	$(CH)chmod 644 $(LIBCRON)/at.allow
	$(CH)chgrp sys $(LIBCRON)/at.allow
	$(TOUCH) 0101000070 $(LIBCRON)/at.allow
	$(CH)chown root $(LIBCRON)/at.allow

cron.allow::
	cp cron.allow $(LIBCRON)/cron.allow
	$(CH)chmod 644 $(LIBCRON)/cron.allow
	$(CH)chgrp sys $(LIBCRON)/cron.allow
	$(TOUCH) 0101000070 $(LIBCRON)/cron.allow
	$(CH)chown root $(LIBCRON)/cron.allow

queuedefs::
	cp queuedefs $(LIBCRON)/queuedefs
	$(CH)chmod 644 $(LIBCRON)/queuedefs
	$(CH)chgrp sys $(LIBCRON)/queuedefs
	$(TOUCH) 0101000070 $(LIBCRON)/queuedefs
	$(CH)chown root $(LIBCRON)/queuedefs


bcheckrc::
	-if vax;\
	then cd vax;\
	elif pdp11;\
	then cd pdp11;\
	elif u3b5;\
	then cd u3b5;\
	elif u3b;\
	then cd u3b;\
		cp bcheckrc.sh $(INSDIR)/bcheckrc;\
		$(CH)chmod 744 $(INSDIR)/bcheckrc;\
		$(CH)chgrp sys $(INSDIR)/bcheckrc;\
		$(TOUCH) 0101000070 $(INSDIR)/bcheckrc;\
		$(CH)chown root $(INSDIR)/bcheckrc;\
	fi

brc::
	-if vax;\
	then cd vax;\
	elif pdp11;\
	then cd pdp11;\
	elif u3b5;\
	then cd u3b5;\
	elif u3b;\
	then cd u3b;\
		cp brc.sh $(INSDIR)/brc;\
		$(CH)chmod 744 $(INSDIR)/brc;\
		$(CH)chgrp sys $(INSDIR)/brc;\
		$(TOUCH) 0101000070 $(INSDIR)/brc;\
		$(CH)chown root $(INSDIR)/brc;\
	fi

checkall::
	-if vax || m68k || pdp11 || u3b5 || u3b;\
	then\
		cp checkall.sh $(INSDIR)/checkall;\
		$(CH)chmod 744 $(INSDIR)/checkall;\
		$(CH)chgrp bin $(INSDIR)/checkall;\
		$(TOUCH) 0101000070 $(INSDIR)/checkall;\
		$(CH)chown bin $(INSDIR)/checkall;\
	fi

checklist::
	-if m68k;\
	then cd m68k;\
	elif vax;\
	then cd vax;\
	elif pdp11;\
	then cd pdp11;\
	elif u3b5;\
	then cd u3b5;\
	elif u3b;\
	then cd u3b;\
		cp checklist $(INSDIR)/checklist;\
		$(CH)chmod 664 $(INSDIR)/checklist;\
		$(CH)chgrp sys $(INSDIR)/checklist;\
		$(TOUCH) 0101000070 $(INSDIR)/checklist;\
		$(CH)chown root $(INSDIR)/checklist;\
	fi

filesave::
	-if vax;\
	then cd vax;\
	elif pdp11;\
	then cd pdp11;\
	elif u3b5;\
	then cd u3b5;\
	elif u3b;\
	then cd u3b;\
		cp filesave.sh $(INSDIR)/filesave;\
		$(CH)chmod 744 $(INSDIR)/filesave;\
		$(CH)chgrp sys $(INSDIR)/filesave;\
		$(TOUCH) 0101000070 $(INSDIR)/filesave;\
		$(CH)chown root $(INSDIR)/filesave;\
	fi

gettydefs::
	-if m68k;\
	then cd m68k;\
	elif vax;\
	then cd vax;\
	elif pdp11;\
	then cd pdp11;\
	elif u3b5;\
	then cd u3b5;\
	elif u3b;\
	then cd u3b;\
	else cd u3b2;\
	fi;\
	cp gettydefs $(INSDIR)/gettydefs;\
	$(CH)chmod 644 $(INSDIR)/gettydefs;\
	$(CH)chgrp sys $(INSDIR)/gettydefs;\
	$(TOUCH) 0101000070 $(INSDIR)/gettydefs;\
	$(CH)chown root $(INSDIR)/gettydefs

group::
	cp group $(INSDIR)/group
	$(CH)chmod 644 $(INSDIR)/group
	$(CH)chgrp sys $(INSDIR)/group
	$(TOUCH) 0101000070 $(INSDIR)/group
	$(CH)chown root $(INSDIR)/group

ioctl.syscon::
	-if m68k;\
	then cd m68k;\
	elif vax;\
	then cd vax;\
	elif pdp11;\
	then cd pdp11;\
	elif u3b5;\
	then cd u3b5;\
	elif u3b;\
	then cd u3b;\
	else cd u3b2;\
	fi;\
	cp ioctl.syscon $(INSDIR)/ioctl.syscon;\
	$(CH)chmod 644 $(INSDIR)/ioctl.syscon;\
	$(CH)chgrp sys $(INSDIR)/ioctl.syscon;\
	$(TOUCH) 0101000070 $(INSDIR)/ioctl.syscon;\
	$(CH)chown root $(INSDIR)/ioctl.syscon

master::
	-if vax;\
	then cd vax;\
	elif pdp11;\
	then cd pdp11;\
	elif u3b5;\
	then cd u3b5;\
	elif u3b;\
	then cd u3b;\
		cp master $(INSDIR)/master;\
		$(CH)chmod 644 $(INSDIR)/master;\
		$(CH)chgrp sys $(INSDIR)/master;\
		$(CH)chown root $(INSDIR)/master;\
	fi

motd::
	-if m68k;\
	then cd m68k;\
	elif vax;\
	then cd vax;\
	elif pdp11;\
	then cd pdp11;\
	elif u3b5;\
	then cd u3b5;\
	elif u3b;\
	then cd u3b;\
	else cd u3b2;\
	fi;\
	sed 1d motd > $(INSDIR)/motd;\
	$(CH)chmod 644 $(INSDIR)/motd;\
	$(CH)chgrp sys $(INSDIR)/motd;\
	$(CH)chown root $(INSDIR)/motd

passwd::
	-if m68k;\
	then cd m68k;\
	elif vax;\
	then cd vax;\
	elif pdp11;\
	then cd pdp11;\
	elif u3b5;\
	then cd u3b5;\
	elif u3b;\
	then cd u3b;\
	else cd u3b2;\
	fi;\
	cp passwd $(INSDIR)/passwd;\
	$(CH)chmod 644 $(INSDIR)/passwd;\
	$(CH)chgrp sys $(INSDIR)/passwd;\
	$(TOUCH) 0101000070 $(INSDIR)/passwd;\
	$(CH)chown root $(INSDIR)/passwd

powerfail::
	-if vax;\
	then cd vax;\
	elif pdp11;\
	then cd pdp11;\
	elif u3b5;\
	then cd u3b5;\
	elif u3b;\
	then cd u3b;\
		cp powerfail.sh $(INSDIR)/powerfail;\
		$(CH)chmod 744 $(INSDIR)/powerfail;\
		$(CH)chgrp sys $(INSDIR)/powerfail;\
		$(TOUCH) 0101000070 $(INSDIR)/powerfail;\
		$(CH)chown root $(INSDIR)/powerfail;\
	fi

rc::
	-if vax;\
	then cd vax;\
	elif pdp11;\
	then cd pdp11;\
	elif u3b5;\
	then cd u3b5;\
	elif u3b;\
	then cd u3b;\
		cp rc.sh $(INSDIR)/rc;\
		$(CH)chmod 744 $(INSDIR)/rc;\
		$(CH)chgrp sys $(INSDIR)/rc;\
		$(TOUCH) 0101000070 $(INSDIR)/rc;\
		$(CH)chown root $(INSDIR)/rc;\
	fi

shutdown::
	-if vax;\
	then cd vax;\
	elif pdp11;\
	then cd pdp11;\
	elif u3b5;\
	then cd u3b5;\
	elif u3b;\
	then cd u3b;\
		cp shutdown.sh $(INSDIR)/shutdown;\
		$(CH)chmod 744 $(INSDIR)/shutdown;\
		$(CH)chgrp sys $(INSDIR)/shutdown;\
		$(TOUCH) 0101000070 $(INSDIR)/shutdown;\
		$(CH)chown root $(INSDIR)/shutdown;\
	fi

tapesave::
	-if vax;\
	then cd vax;\
	elif pdp11;\
	then cd pdp11;\
	elif u3b5;\
	then cd u3b5;\
	elif u3b;\
	then cd u3b;\
		cp tapesave.sh $(INSDIR)/tapesave;\
		$(CH)chmod 744 $(INSDIR)/tapesave;\
		$(CH)chgrp sys $(INSDIR)/tapesave;\
		$(TOUCH) 0101000070 $(INSDIR)/tapesave;\
		$(CH)chown root $(INSDIR)/tapesave;\
	fi

system::
	-if u3b;\
	then cd u3b;\
		cp system $(INSDIR)/system;\
		$(CH)chmod 644 $(INSDIR)/system;\
		$(CH)chgrp sys $(INSDIR)/system;\
		$(TOUCH) 0101000070 $(INSDIR)/system;\
		$(CH)chown root $(INSDIR)/system;\
	fi

system.mtc11::
	-if u3b;\
	then cd u3b;\
		cp system.32 $(INSDIR)/system.mtc11;\
		$(CH)chmod 644 $(INSDIR)/system.mtc11;\
		$(CH)chgrp sys $(INSDIR)/system.mtc11;\
		$(TOUCH) 0101000070 $(INSDIR)/system.mtc11;\
		$(CH)chown root $(INSDIR)/system.mtc11;\
	fi

system.mtc12::
	-if u3b;\
	then cd u3b;\
		cp system.32 $(INSDIR)/system.mtc12;\
		$(CH)chmod 644 $(INSDIR)/system.mtc12;\
		$(CH)chgrp sys $(INSDIR)/system.mtc12;\
		$(TOUCH) 0101000070 $(INSDIR)/system.mtc12;\
		$(CH)chown root $(INSDIR)/system.mtc12;\
	fi

system.un32::
	-if u3b;\
	then cd u3b;\
		cp system.32 $(INSDIR)/system.un32;\
		$(CH)chmod 644 $(INSDIR)/system.un32;\
		$(CH)chgrp sys $(INSDIR)/system.un32;\
		$(TOUCH) 0101000070 $(INSDIR)/system.un32;\
		$(CH)chown root $(INSDIR)/system.un32;\
	fi

#
# sysV68 modifications
#

FT45DL::
	-if /bin/m68k;\
	then cd m68k;\
	fi;\
	cp FT45DL $(LIB)/term/FT45DL;\
	chmod 664 $(LIB)/term/FT45DL;\
	chgrp bin $(LIB)/term/FT45DL;\
	$(TOUCH) 0101000070 $(LIB)/term/FT45DL;\
	chown bin $(LIB)/term/FT45DL

fmtflp::
	-if /bin/m68k;\
	then cd m68k;\
	fi;\
	cp fmtflp.sh $(INSDIR)/fmtflp;\
	cp fmtbflp.sh $(INSDIR)/fmtbflp;\
	chmod 755 $(INSDIR)/fmtflp $(INSDIR)/fmtbflp;\
	chgrp sys $(INSDIR)/fmtflp $(INSDIR)/fmtbflp;\
	$(TOUCH) 0101000070 $(INSDIR)/fmtflp $(INSDIR)/fmtbflp;\
	chown root $(INSDIR)/fmtflp $(INSDIR)/fmtbflp

perms::
	-if /bin/m68k;\
	then cd m68k;\
	fi;\
	cp perms.sh /etc/perms;\
	chmod 644 /etc/perms;\
	chgrp sys /etc/perms;\
	$(TOUCH) 0101000070 /etc/perms;\
	chown root /etc/perms


mklost+found::
	-if /bin/m68k;\
	then cd m68k;\
	fi;\
	cp mklost+found $(INSDIR);\
	chmod 755 $(INSDIR)/mklost+found;\
	chgrp sys $(INSDIR)/mklost+found;\
	$(TOUCH) 0101000070 $(INSDIR)/mklost+found;\
	chown root $(INSDIR)/mklost+found

termcap::
	-if /bin/m68k;\
	then cd m68k;\
	fi;\
	cp termcap $(INSDIR)/termcap;\
	chmod 644 $(INSDIR)/termcap;\
	chgrp bin $(INSDIR)/termcap;\
	$(TOUCH) 0101000070 $(INSDIR)/termcap;\
	chown bin $(INSDIR)/termcap

termsupport::
	-if /bin/m68k;\
	then cd m68k;\
	fi;\
	if [ ! -d $(LOCBIN) ];\
	then mkdir $(LOCBIN);\
	fi;\
	cp TermAssume.sh $(LOCBIN)/TermAssume;\
	cp TermSetup.sh $(LOCBIN)/TermSetup;\
	cp TermFuncs.sh $(LOCBIN)/TermFuncs;\
	cp TermIs.sh $(LOCBIN)/TermIs;\
	chmod 755 $(LOCBIN)/TermSetup $(LOCBIN)/TermFuncs $(LOCBIN)/TermIs;\
	chgrp sys $(LOCBIN)/TermSetup $(LOCBIN)/TermFuncs $(LOCBIN)/TermIs;\
	$(TOUCH) 0101000070 $(LOCBIN)/TermSetup $(LOCBIN)/TermFuncs;\
	$(TOUCH) 0101000070 $(LOCBIN)/TermIs;\
	chown root $(LOCBIN)/TermSetup $(LOCBIN)/TermFuncs $(LOCBIN)/TermIs

tm220.fkey::
	-if /bin/m68k;\
	then cd m68k;\
	fi;\
	cp tm220.fkey $(LIB)/term/tm220.fkey;\
	chmod 644 $(LIB)/term/tm220.fkey;\
	chgrp sys $(LIB)/term/tm220.fkey;\
	$(TOUCH) 0101000070 $(LIB)/term/tm220.fkey;\
	chown root $(LIB)/term/tm220.fkey

diskinfo::
	-if /bin/m68k;\
	then cd m68k;\
		cp diskdefs $(INSDIR)/diskdefs;\
		chmod 644 $(INSDIR)/diskdefs;\
		chgrp sys $(INSDIR)/diskdefs;\
		$(TOUCH) 0101000070 $(INSDIR)/diskdefs;\
		chown root $(INSDIR)/diskdefs;\
		if [ ! -d $(INSDIR)/diskalts ] ;\
		then\
			mkdir $(INSDIR)/diskalts;\
			chmod 775 $(INSDIR)/diskalts;\
		fi;\
		for i in m320fix70* m32015 m32040* m32070* m323* m320140 m360*337 m360168*;\
		do\
			cp $$i $(INSDIR)/diskalts/$$i;\
			chmod 644 $(INSDIR)/diskalts/$$i;\
			chgrp sys $(INSDIR)/diskalts/$$i;\
			$(TOUCH) 0101000070 $(INSDIR)/diskalts/$$i;\
			chown root $(INSDIR)/diskalts/$$i;\
		done ;\
	fi

syslist::
	-if /bin/m68k;\
	then cd m68k;\
	fi;\
	cp syslist $(INSDIR)/syslist;\
	chmod 644 $(INSDIR)/syslist;\
	chgrp sys $(INSDIR)/syslist;\
	$(TOUCH) 0101000070 $(INSDIR)/syslist;\
	chown root $(INSDIR)/syslist

.profile::
	-if /bin/m68k;\
	then cd m68k;\
	fi;\
	cp slashprfle.sh $(ROOT)/.profile;\
	chmod 644 $(ROOT)/.profile;\
	chgrp sys $(ROOT)/.profile;\
	$(TOUCH) 0101000070 $(ROOT)/.profile;\
	chown root $(ROOT)/.profile
