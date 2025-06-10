#
#ident	"@(#)libcmcnet:libcmcnet.mk	1.10"
#
ROOT =
CC = cc
LIBDIR = .
INCDIR = $(ROOT)/usr/include
CINCDIR = $(ROOT)/usr/include/CMC
USRLIB = $(ROOT)/usr/lib
LIBNAME = libcmcnet.a
LLIB = cmcnet
LINTLIB = llib-l$(LLIB).ln
PROFILE=
DEBUG=
CFLAGS=-O -I $(INCDIR) $(DEBUG) $(PROFILE)

SRC =\
	rcmd.c\
	connect.c\
	gethostbyadd.c\
	getnetbyaddr.c\
	getnetbyname.c\
	getnetent.c\
	getproto.c\
	getprotoname.c\
	getprotoent.c\
	getservbynam.c\
	getservbypor.c\
	getservent.c\
	inet_lnaof.c\
	inet_makeadd.c\
	inet_netof.c\
	inet_network.c\
	inet_ntoa.c\
	accept.c\
	getpeername.c\
	bind.c\
	gethostbynam.c\
	gethostent.c\
	inet_addr.c\
	listen.c\
	sockopt.c\
	shutdown.c\
	socket.c\
	gethostname.c\
	opendir.c\
	readdir.c\
	ruserpass.c\
	send.c\
	recv.c\
	net_error.c\
	perror.c\
	sendto.c\
	recvfrom.c getcwd.c

FILES =\
	$(LIBNAME)(rcmd.o)\
	$(LIBNAME)(connect.o)\
	$(LIBNAME)(gethostbyadd.o)\
	$(LIBNAME)(getnetbyaddr.o)\
	$(LIBNAME)(getnetbyname.o)\
	$(LIBNAME)(getnetent.o)\
	$(LIBNAME)(getproto.o)\
	$(LIBNAME)(getprotoname.o)\
	$(LIBNAME)(getprotoent.o)\
	$(LIBNAME)(getservbynam.o)\
	$(LIBNAME)(getservbypor.o)\
	$(LIBNAME)(getservent.o)\
	$(LIBNAME)(inet_lnaof.o)\
	$(LIBNAME)(inet_makeadd.o)\
	$(LIBNAME)(inet_netof.o)\
	$(LIBNAME)(inet_network.o)\
	$(LIBNAME)(inet_ntoa.o)\
	$(LIBNAME)(accept.o)\
	$(LIBNAME)(getpeername.o)\
	$(LIBNAME)(bind.o)\
	$(LIBNAME)(gethostbynam.o)\
	$(LIBNAME)(gethostent.o)\
	$(LIBNAME)(inet_addr.o)\
	$(LIBNAME)(listen.o)\
	$(LIBNAME)(sockopt.o)\
	$(LIBNAME)(shutdown.o)\
	$(LIBNAME)(socket.o)\
	$(LIBNAME)(gethostname.o)\
	$(LIBNAME)(opendir.o)\
	$(LIBNAME)(readdir.o)\
	$(LIBNAME)(ruserpass.o)\
	$(LIBNAME)(send.o)\
	$(LIBNAME)(recv.o)\
	$(LIBNAME)(net_error.o)\
	$(LIBNAME)(perror.o)\
	$(LIBNAME)(sendto.o)\
	$(LIBNAME)(recvfrom.o) $(LIBNAME)(getcwd.o)

lib:	$(LIBNAME) 

lint:
	lint -uax -o $(LLIB) $(SRC)
install: lib
	cp $(LIBNAME) $(USRLIB)
uninstall:
	-rm $(USRLIB)/$(LIBNAME)

.PRECIOUS:	$(LIBNAME)

$(LIBNAME):	$(FILES)

clean:
	-rm -f *.o

clobber: clean
	-rm -f $(LIBNAME)

#### dependencies now follow

$(LIBNAME)(gethostbyadd.o):$(CINCDIR)/longnames.h $(CINCDIR)/netdb.h
$(LIBNAME)(gethostbynam.o):$(CINCDIR)/longnames.h $(CINCDIR)/netdb.h
$(LIBNAME)(gethostent.o):$(CINCDIR)/longnames.h $(CINCDIR)/types.h \
	$(CINCDIR)/socket.h $(CINCDIR)/netdb.h
$(LIBNAME)(gethostname.o):$(CINCDIR)/longnames.h
$(LIBNAME)(getnetbyaddr.o):$(CINCDIR)/longnames.h $(CINCDIR)/netdb.h
$(LIBNAME)(getnetbyname.o):$(CINCDIR)/longnames.h $(CINCDIR)/netdb.h
$(LIBNAME)(getnetent.o):$(CINCDIR)/longnames.h $(CINCDIR)/types.h \
	$(CINCDIR)/socket.h $(CINCDIR)/netdb.h
$(LIBNAME)(getproto.o):$(CINCDIR)/longnames.h $(CINCDIR)/netdb.h
$(LIBNAME)(getprotoent.o):$(CINCDIR)/longnames.h $(CINCDIR)/types.h \
	$(CINCDIR)/socket.h $(CINCDIR)/netdb.h
$(LIBNAME)(getprotoname.o):$(CINCDIR)/longnames.h $(CINCDIR)/netdb.h
$(LIBNAME)(getservbynam.o):$(CINCDIR)/longnames.h $(CINCDIR)/netdb.h
$(LIBNAME)(getservbypor.o):$(CINCDIR)/longnames.h $(CINCDIR)/netdb.h
$(LIBNAME)(getservent.o):$(CINCDIR)/longnames.h $(CINCDIR)/types.h \
	$(CINCDIR)/socket.h $(CINCDIR)/netdb.h $(CINCDIR)/in.h
$(LIBNAME)(inet_addr.o):$(CINCDIR)/longnames.h $(CINCDIR)/types.h $(CINCDIR)/in.h
$(LIBNAME)(inet_lnaof.o):$(CINCDIR)/longnames.h $(CINCDIR)/types.h $(CINCDIR)/in.h
$(LIBNAME)(inet_makeadd.o):$(CINCDIR)/longnames.h $(CINCDIR)/types.h \
	$(CINCDIR)/in.h
$(LIBNAME)(inet_netof.o):$(CINCDIR)/longnames.h $(CINCDIR)/types.h $(CINCDIR)/in.h
$(LIBNAME)(inet_network.o):$(CINCDIR)/longnames.h $(CINCDIR)/types.h
$(LIBNAME)(inet_ntoa.o):$(CINCDIR)/longnames.h $(CINCDIR)/types.h $(CINCDIR)/in.h
$(LIBNAME)(net_error.o):$(CINCDIR)/errno.h
$(LIBNAME)(opendir.o):$(CINCDIR)/types.h $(CINCDIR)/dir.h
$(LIBNAME)(perror.o):$(CINCDIR)/errno.h
$(LIBNAME)(rcmd.o):$(CINCDIR)/longnames.h $(CINCDIR)/types.h \
	$(CINCDIR)/socket.h $(CINCDIR)/in.h $(CINCDIR)/netdb.h \
	$(CINCDIR)/errno.h
$(LIBNAME)(readdir.o):$(CINCDIR)/types.h $(CINCDIR)/dir.h
$(LIBNAME)(ruserpass.o):$(CINCDIR)/types.h $(CINCDIR)/errno.h
