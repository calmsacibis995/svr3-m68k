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
#include <sys/CMC/in.h>
#include <sys/CMC/enp.h>
#include <sys/param.h>
#include <sys/signal.h>
#include <sys/fs/s5dir.h>
#include <sys/user.h>


/*
 *		if_rip.c
 *
 *	RIP specific routines - Called through protocol switch inetsw[].
 *	We use sockaddr_in.sin_port to hold the upper-level protocol number
 *	for the Raw Internet interface.
 *
 *	ripnmchk()
 *	ripbind()
 *	ripconnect()
 *	ripclose()
 *	ripsend()
 *	riprecv()
 */


extern
SHARED_RINGS	*enp;

extern CHAN	*data_chan;

extern HOST	MYHOSTPARAM;

static
sockaddr_in	sin = {AF_INET};


ripnmchk(s)
register SOCKET	*s;
{
	return (0);
}


ripbind(s)
register SOCKET	*s;
{
	register sockaddr_in	*sin, *sin2;

	if ((s->so_state & SS_RESV) == 0)
		return (EINVAL);

	sin2 = (sockaddr_in *) (data_chan[(short) s->so_enpid].c_name) + 1;
	sin = (sockaddr_in *) s->so_name;
	enp_assign(sin2->sin_addr.s_addr, htonl(sin->sin_addr.s_addr));
	sin2->sin_port = htons(IPPROTO_RAW);
	return (0);
}


ripconnect(s, to, type, flags)
register SOCKET		*s;
register sockaddr_in	*to;
{
	register sockaddr_in	*sin;

	if (to->sin_addr.s_addr == 0) {
		u.u_error = EINVAL;
		return;
	}

	/*	Set foreign host/protocol in channel	*/

	sin = (sockaddr_in *) data_chan[(short) s->so_enpid].c_name;
	enp_assign(sin->sin_addr.s_addr, htonl(to->sin_addr.s_addr));
	sin->sin_port = htons(IPPROTO_RAW);

	/*	Set foreign host/protocol in socket		*/

	sin = (sockaddr_in *) (s->so_name) + 1;
	sin->sin_addr.s_addr = to->sin_addr.s_addr;
	sin->sin_port = IPPROTO_RAW;

	s->so_state |= SS_CONNECTED;

	return (0);
}


ripclose(s)
register SOCKET	*s;
{
	register CHAN	*cp;

	cp = &data_chan[(short) s->so_enpid];

	while (htons(cp->c_inflight) > 0);
	enp_zero(cp->c_name, SOCKNAMESIZE);
	cp->c_state = SS_AVAIL;
}


ripsend(s)
register SOCKET	*s;
{
	register sockaddr_in	*sin;

	sin = (sockaddr_in *) (s->so_name) + 1;
	return ((int) sin);
}


riprecv(s)
register SOCKET	*s;
{
	return (0);
}
