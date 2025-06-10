/*	Copyright (c) 1984 by Communication Machinery Corporation
 *
 *	This file contains material which is proprietary to
 *	Communication Machinery Corporation (CMC) and which
 *	may not be divulged without the written permission
 *	of CMC.
 */


#include <sys/CMC/types.h>
#include <sys/CMC/errno.h>
#include <sys/CMC/ring.h>
#include <sys/CMC/netbfr.h>
#include <sys/CMC/CMC_proto.h>
#include <sys/CMC/CMC_socket.h>
#include <sys/CMC/eth_proto.h>
#include <sys/CMC/eth.h>
#include <sys/CMC/enp.h>
#include <sys/param.h>
#include <sys/signal.h>
#include <sys/fs/s5dir.h>
#include <sys/user.h>


/*
 *		if_eth.c
 *
 *	Raw Ethernet routines - Called through protocol switch ethsw[].
 *	We use sockaddr_in.sin_port to hold the Ethernet packet type
 *	for the Raw Ethernet interface.
 *
 *	ethbind()
 *	ethconnect()
 *	ethclose()
 *	ethsend()
 *	ethrecv()
 */


extern
SHARED_RINGS	*enp;

extern CHAN	*data_chan;


ethbind(s)
register SOCKET	*s;
{
	register sockaddr_eth	*sen, *sen2;

	if ((s->so_state & SS_RESV) == 0)
		return (EINVAL);

	sen2 = (sockaddr_eth *) (data_chan[(short) s->so_enpid].c_name) + 1;
	sen = (sockaddr_eth *) s->so_name;
	enp_move(&sen->sen_addr, &sen2->sen_addr, sizeof(en_addr));
	sen2->sen_type = htons(sen->sen_type);

	s->so_state |= SS_CONNECTED;
	return (0);
}


ethconnect(s, to, type, flags)
register SOCKET		*s;
register sockaddr_eth	*to;
{
	return (0);
}


ethclose(s)
register SOCKET	*s;
{
	register CHAN	*cp;

	cp = &data_chan[(short) s->so_enpid];

	while (htons(cp->c_inflight) > 0);
	enp_zero(cp->c_name, SOCKNAMESIZE);
	cp->c_state = SS_AVAIL;
}


ethsend(s)
register SOCKET	*s;
{
	return (0);
}


ethrecv(s)
register SOCKET	*s;
{
	return (0);
}
