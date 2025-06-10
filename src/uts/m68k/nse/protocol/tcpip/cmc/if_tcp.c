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
 *		if_tcp.c
 *
 *	TCP specific routines - Called through protocol switch inetsw[].
 *
 *	tcpbind()
 *	tcpconnect()
 *	tcpclose()
 *	tcpsend()
 *	tcprecv()
 */


extern
SHARED_RINGS	*enp;

extern CHAN	*data_chan;
extern HOST	MYHOSTPARAM;

static
sockaddr_in	sin = {AF_INET};


tcpbind(s)
register SOCKET	*s;
{
	return (0);
}


tcpconnect(s, to, type, flags)
register SOCKET		*s;
register sockaddr_in	*to;
{
	register BFR	*bp;
	register OPN	*op;

	extern		wait_for();

	if ((to) && (to->sin_port == 0) || (to->sin_addr.s_addr == 0)) {
		u.u_error = EINVAL;	/* ? */
		return;
	}

	/* if flags set then don't wait in get_buf */
	if ((bp = (BFR *) get_buf(flags)) == NULL) {
		return(1);
	}
	bp->b_device = s->so_proto->pr_channel;

	op = (OPN *) enp_rval(bp->b_addr);
	enp_move(s->so_name, op->o_name, SOCKNAMESIZE);
	enp_assign(op->o_id, htonl(s->so_enpid));
	op->o_status = -1;
	op->o_type = htons(type);

	if (to)
		enp_move(to, op->o_parm, SOCKNAMESIZE);
	else
		enp_zero(op->o_parm, SOCKNAMESIZE);

	s->so_o_bfr = bp;		/* Save open buffer and		*/
	s->so_o_opn = op;		/* address of open parm in bfr	*/
	s->so_state |= SS_CONNECTING;

	to_enp(enp->h_open, bp);

	if ((type != SS_LISTEN) && (flags != -1))
		u.u_error = wait_for(s, flags);

	return(0);
}


tcpclose(s)
register SOCKET	*s;
{
	register BFR	*bp;
	register int	x;

	bp = (BFR *) get_buf(0);
	bp->b_device = s->so_enpid;
	bp->b_flags = (SC_CLOSE | SC_CTL);
	bp->b_resv = 0;

	to_enp(enp->h_tonet, bp);

	if (s->so_options & SO_LINGER) {
		x = splnet();
		while (bp->b_resv == 0)
			sleep(bp, PENP);
		splx(x);
		fre_buf(bp);
	}
}


tcpsend(s)
register SOCKET	*s;
{
	return (0);
}


tcprecv(s)
register SOCKET	*s;
{
	return (0);
}
