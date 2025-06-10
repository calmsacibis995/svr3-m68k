# @(#)Makefile	1.0 
LIBNAME = ../../../../cf/lib.tcpip
INCRT = ../../../..
FRC = 

CFLAGS = -c -O -I. -I$(INCRT) -DINKERNEL $(RFSDBUG) 

FILES =\
	$(LIBNAME)(enp.o)\
	$(LIBNAME)(enp10.o)\
	$(LIBNAME)(enpram.o)\
	$(LIBNAME)(eth_proto.o)\
	$(LIBNAME)(if_eth.o)\
	$(LIBNAME)(if_rip.o)\
	$(LIBNAME)(if_tcp.o)\
	$(LIBNAME)(if_udp.o)\
	$(LIBNAME)(in_proto.o)\
	$(LIBNAME)(memcmp.o)\
	$(LIBNAME)(memcpy.o)\
	$(LIBNAME)(pty.o)\
	$(LIBNAME)(ring.o)\
	$(LIBNAME)(sock_driver.o)

all:	$(LIBNAME) 

.PRECIOUS:	$(LIBNAME)

$(LIBNAME):	$(FILES)

.c.a:
	$(CC) $(CFLAGS) $<
	chmod 664 $*.o
	ar rv $@ $*.o
	-rm -f $*.o

clean:
	-rm -f *.o

clobber:	clean
	-rm -f $(LIBNAME)

$(LIBNAME)(enp.o): enp.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/CMC/types.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/CMC/errno.h \
	$(INCRT)/sys/CMC/signal.h \
	$(INCRT)/sys/CMC/enp_addr.h \
	$(INCRT)/sys/CMC/netbfr.h \
	$(INCRT)/sys/CMC/ring.h \
	$(INCRT)/sys/CMC/CMC_proto.h \
	$(INCRT)/sys/CMC/CMC_socket.h \
	$(INCRT)/sys/CMC/in.h \
	$(INCRT)/sys/CMC/enp.h \
	$(FRC)
$(LIBNAME)(enp10.o): enp10.c \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/inode.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/CMC/errno.h \
	$(INCRT)/sys/CMC/signal.h \
	$(INCRT)/sys/CMC/enp_addr.h \
	$(INCRT)/sys/CMC/netbfr.h \
	$(INCRT)/sys/CMC/ring.h \
	$(INCRT)/sys/CMC/CMC_proto.h \
	$(INCRT)/sys/CMC/CMC_socket.h \
	$(INCRT)/sys/CMC/copyin.h \
	$(FRC)
$(LIBNAME)(eth_proto.o): eth_proto.c \
	$(INCRT)/sys/CMC/types.h \
	$(INCRT)/sys/CMC/errno.h \
	$(INCRT)/sys/CMC/ring.h \
	$(INCRT)/sys/CMC/netbfr.h \
	$(INCRT)/sys/CMC/CMC_proto.h \
	$(INCRT)/sys/CMC/CMC_socket.h \
	$(INCRT)/sys/CMC/eth_proto.h \
	$(INCRT)/sys/CMC/eth.h \
	$(INCRT)/sys/CMC/enp.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/user.h \
	$(FRC)
$(LIBNAME)(if_eth.o): if_eth.c \
	$(INCRT)/sys/CMC/types.h \
	$(INCRT)/sys/CMC/errno.h \
	$(INCRT)/sys/CMC/ring.h \
	$(INCRT)/sys/CMC/netbfr.h \
	$(INCRT)/sys/CMC/CMC_proto.h \
	$(INCRT)/sys/CMC/CMC_socket.h \
	$(INCRT)/sys/CMC/eth_proto.h \
	$(INCRT)/sys/CMC/eth.h \
	$(INCRT)/sys/CMC/enp.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/user.h \
	$(FRC)
$(LIBNAME)(if_rip.o): if_rip.c \
	$(INCRT)/sys/CMC/types.h \
	$(INCRT)/sys/CMC/errno.h \
	$(INCRT)/sys/CMC/ring.h \
	$(INCRT)/sys/CMC/netbfr.h \
	$(INCRT)/sys/CMC/CMC_proto.h \
	$(INCRT)/sys/CMC/CMC_socket.h \
	$(INCRT)/sys/CMC/in.h \
	$(INCRT)/sys/CMC/enp.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/user.h \
	$(FRC)
$(LIBNAME)(if_tcp.o): if_tcp.c \
	$(INCRT)/sys/CMC/types.h \
	$(INCRT)/sys/CMC/errno.h \
	$(INCRT)/sys/CMC/ring.h \
	$(INCRT)/sys/CMC/netbfr.h \
	$(INCRT)/sys/CMC/CMC_proto.h \
	$(INCRT)/sys/CMC/CMC_socket.h \
	$(INCRT)/sys/CMC/in.h \
	$(INCRT)/sys/CMC/enp.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/user.h \
	$(FRC)
$(LIBNAME)(if_udp.o): if_udp.c \
	$(INCRT)/sys/CMC/types.h \
	$(INCRT)/sys/CMC/errno.h \
	$(INCRT)/sys/CMC/ring.h \
	$(INCRT)/sys/CMC/netbfr.h \
	$(INCRT)/sys/CMC/CMC_proto.h \
	$(INCRT)/sys/CMC/CMC_socket.h \
	$(INCRT)/sys/CMC/in.h \
	$(INCRT)/sys/CMC/enp.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/user.h \
	$(FRC)
$(LIBNAME)(in_proto.o): in_proto.c \
	$(INCRT)/sys/CMC/types.h \
	$(INCRT)/sys/CMC/errno.h \
	$(INCRT)/sys/CMC/ring.h \
	$(INCRT)/sys/CMC/netbfr.h \
	$(INCRT)/sys/CMC/CMC_proto.h \
	$(INCRT)/sys/CMC/CMC_socket.h \
	$(INCRT)/sys/CMC/in.h \
	$(INCRT)/sys/CMC/enp.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/user.h \
	$(FRC)
$(LIBNAME)(memcmp.o): memcmp.c \
	$(FRC)
$(LIBNAME)(memcpy.o): memcpy.c \
	$(FRC)
$(LIBNAME)(pty.o): pty.c \
	$(INCRT)/sys/types.h \
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/CMC/errno.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/termio.h \
	$(INCRT)/sys/conf.h \
	$(INCRT)/sys/tty.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/CMC/pty.h \
	$(INCRT)/sys/CMC/copyin.h \
	$(FRC)
$(LIBNAME)(ring.o):ring.c\
	$(INCRT)/sys/CMC/ring.h \
	$(FRC)
$(LIBNAME)(sock_driver.o):sock_driver.c\
	$(INCRT)/sys/param.h \
	$(INCRT)/sys/sysmacros.h \
	$(INCRT)/sys/CMC/types.h \
	$(INCRT)/sys/fs/s5dir.h \
	$(INCRT)/sys/CMC/signal.h \
	$(INCRT)/sys/user.h \
	$(INCRT)/sys/proc.h \
	$(INCRT)/sys/file.h \
	$(INCRT)/sys/CMC/longnames.h \
	$(INCRT)/sys/CMC/netbfr.h \
	$(INCRT)/sys/CMC/ring.h \
	$(INCRT)/sys/CMC/CMC_proto.h \
	$(INCRT)/sys/CMC/CMC_socket.h \
	$(INCRT)/sys/CMC/in.h \
	$(INCRT)/sys/CMC/enp.h \
	$(INCRT)/sys/CMC/user_socket.h \
	$(INCRT)/sys/CMC/errno.h \
	$(FRC)

FRC:
