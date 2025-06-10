/*	Copyright (c) 1984 by Communication Machinery Corporation
 *
 *	This file contains material which is proprietary to
 *	Communication Machinery Corporation (CMC) and which
 *	may not be divulged without the written permission
 *	of CMC.
 */

/*
 *	File: sock_driver.c	Module(s): HOST Resident
 *
 *	This file implements a device driver for a CMC
 *	ENP.  The device driver supports the semantics
 *	of the Berkeley 4.2BSD socket abstraction as applied
 *	to the AF_INET domain.
 *
 *	Most of the socket-related support work is handled
 *	by code running on the ENP; only enough mechanism remains
 *	to allow simple questions to be answered simply.
 *
 *	The design is an overweight kluge, requiring one special
 *	file per socket.  The big advantage of this is that it
 *	will work on all (non-4.2) Unix systems; for 4.2 systems
 *	there is a different driver that replaces the kernel
 *	support for the AF_INET protocol suite.
 */

/*	HISTORY:
		initial coding July 1984 by John Mullen
		Added generic DMA capability -- 12/18/84 by jrm
		Rev_B Modified - CMC Staff - 4/85
		Changed Channel allocation to tcp 27 udp 2 rip/raw 1
 */

#include <sys/param.h>
#include <sys/sysmacros.h>
#include <sys/CMC/types.h>
#include <sys/fs/s5dir.h>
#include <sys/CMC/signal.h>
#include <sys/user.h>
#include <sys/immu.h>
#include <sys/region.h>
#include <sys/proc.h>
#include <sys/file.h>
#include <sys/CMC/longnames.h>
#include <sys/CMC/netbfr.h>
#include <sys/CMC/ring.h>
#include <sys/CMC/CMC_proto.h>
#include <sys/CMC/CMC_socket.h>
#include <sys/CMC/in.h>
#include <sys/CMC/enp.h>
#include <sys/CMC/user_socket.h>
#include <sys/CMC/errno.h>


/*
 *	Initialization & Interface Globals
 */

HOSTCTL		 host;

extern
SHARED_RINGS	*enp;			/* Pointers to Host/ENP rings	*/
extern CHAN	*data_chan;		/* Data channel array		*/
extern enp_in_system;			/* card available for use       */

static
struct domain	*domains;


/*
 *	Define which protocols we want initialized (both host and ENP)
 *	and the channel allocations for them.
 */

PIB	p_list[] = {
	{ PF_INET, SOCK_STREAM, 0, 26, "if_tcp" },
	{ PF_INET, SOCK_DGRAM, 27, 2, "if_udp" },
	{ PF_INET, SOCK_RAW, 30, 1, "if_rip" },
	{ PF_ETHER, SOCK_RAW, 31, 1, "if_eth" },
	{ 0, 0, 0, 0, 0, }
};


/*
 *	Local macros Etc.
 */

#define min(a,b) ((a)<(b)?(a):(b))
#define	TCPURGENT	0x40		/* HACK!! - so we don't include
					   net.h, internet.h, tcp.h !	*/


/*
 *	Locally declared external variables
 */

SOCKET	sock_table[NSOCKETS];
int chan_inuse = 0;


/*
 *		The CMC Socket Device Driver
 */

so_open(dev, flags)
{
	register SOCKET *s;

	if (!(enp_in_system)) {
		u.u_error = ENXIO;
		return;
	}

	if (minor(dev) >= NSOCKETS) { 
		u.u_error = ENODEV;
		return;
	}

	if ((host.h_state & HS_INIT) == 0) {	/* ENP not inited	*/
		domaininit();		/* Set up protocol struct FIRST	*/
		init_enp(0, p_list);	/* Init ENP & protos we want	*/
	}

	s = &sock_table[(short) minor(dev)];

	if (s->so_state & (SS_RESV | SS_OPEN))
		u.u_error = EEXIST;
	else { 
		enp_zero(s, sizeof(SOCKET));
		s->so_state = SS_OPEN;
	}
}


so_close(dev)
{
	register SOCKET	*s;
	register CHAN	*cp;
	register BFR	*bp;
	register int	x;

	s = &sock_table[(short) minor(dev)];

	if ((s->so_state & SS_OPEN) == 0) { 
		u.u_error = EBADF;
		return;
	}

	if ((s->so_state & SS_RESV) == 0) {	/* No channel allocated	*/
		s->so_state = 0;
		return;
	}

	cp = &data_chan[(short) s->so_enpid];
	cp->c_state = htons(ntohs(cp->c_state) | SS_FLUSHING);

	(*s->so_proto->pr_close)(s);

	x = splnet();
	enpservice();
	splx(x);

	while ((bp = (BFR *) ringget(&cp->c_tohost))) {
		to_enp(enp->e_hostcomplete, bp);
		sringget(&cp->c_sring);
	}

	--chan_inuse;
	s->so_state = 0;			/* Free the socket	*/
}


so_read(dev)
{
	register SOCKET *s;

	s = &sock_table[(short) minor(dev)];

	if ((s->so_state & SS_CONNECTED) == 0) {
		u.u_error = EBADF;
		return;
	}

	sorecv(s, 0);
}


static
sorecv(s, flag)
register SOCKET *s;
{
	return (recvit(s, 0, 0, flag));
}


static
recvit(s, from, fromlen, flag)
register SOCKET	*s;
register char	*from;
register int	*fromlen;
{
	register BFR	*bp;
	register CHAN	*cp;
	register int	x,i, rcvd;

	if (s->so_state & SS_NOREAD) {
		u.u_error = ESHUTDOWN;
		return (-1);
	}

	cp = (CHAN *) &data_chan[(short) s->so_enpid];

	i = splnet();
	enpservice();
	splx(i);

	rcvd = 0;
	while (u.u_count != 0) {
		i = splnet();
		while ((bp = (BFR *) fir(&cp->c_tohost)) == 0) {
			if ((rcvd) || (ntohs(cp->c_state) & SS_BROKEN)) {
				splx(i);
				return (rcvd);
			}
			if (u.u_fmode & FNDELAY) {
				u.u_error = EWOULDBLOCK;
				splx(i);
				return (-1);
			}
			sleep(&cp->c_tohost, PENP);
		}
		splx(i);

		if (i = min(bp->b_msglen, u.u_count)) {
			if (kern_to_user(enp_rval(bp->b_addr), u.u_base, i)) {
				u.u_error = EFAULT;
				return (-1);
			}
			if (from) {
				suword(fromlen, sizeof(sockaddr));
				if(kern_to_user((bp+1),from,sizeof(sockaddr))){
					u.u_error = EFAULT;
					return (-1);
				}
			}
			s->so_state &= ~SS_ATMARK;
			rcvd += i;
		}

		if ((bp->b_msglen -= i) == 0) {
			if (bp->b_flags & TCPURGENT) {	/* OOB	*/
				s->so_state |= SS_ATMARK;
				bp->b_flags &= ~TCPURGENT;
			}
			to_enp(enp->e_hostcomplete, bp);
			x = splnet();
			ringget(&cp->c_tohost);
			sringget(&cp->c_sring);
			splx(x);
			if (s->so_state & SS_ATMARK) {
				if (i == 0) {
					i = -1;
					u.u_error = EINTR;
				}
				return (i);
			}
		}
		else
			enp_assign(bp->b_addr, enp_rval(bp->b_addr) + i);

		u.u_base += i;
		u.u_count -= i;
	}
	return (rcvd);
}


so_write(dev)
{
	register SOCKET *s;

	s = &sock_table[(short) minor(dev)];

	if ((s->so_state & SS_CONNECTED) == 0) {
		u.u_error = EBADF;
		return;
	}

	sosend(s, 0);
}


static
sosend(s, flag)
register SOCKET *s;
{
	register sockaddr	*to;

	to = (sockaddr *) (*s->so_proto->pr_send)(s);
	return (sendit(s, to, flag));
}


static
sendit(s, to, flag)
register SOCKET	*s;
register char	*to;
{
	register BFR	*bp;
	register PROTO	*pr;
	int		sent, i;

	if (s->so_state & SS_NOWRITE) {
		u.u_error = ESHUTDOWN;
		return (-1);
	}

	if (ntohs(data_chan[(short) s->so_enpid].c_state) & SS_BROKEN) {
		u.u_error = EPIPE;
		psignal(u.u_procp, SIGPIPE);
		return (-1);
	}

	i = splnet();
	enpservice();
	splx(i);

	pr = s->so_proto;

	sent = 0;
	while (u.u_count != 0) {
		if ((bp = (BFR *) get_buf(u.u_fmode & FNDELAY)) == 0) {
			u.u_error = EWOULDBLOCK;
			return (-1);
		}

		if (to)
			enp_move(to, enp_rval(bp->b_addr), sizeof(sockaddr));

		i = min(u.u_count, pr->pr_maxdata);
		if (user_to_kern(u.u_base, enp_rval(bp->b_addr) + pr->pr_offset, i)) {
			fre_buf(bp);
			u.u_error = EFAULT;
			return (-1);
		}

		bp->b_flags = flag & MSG_OOB ? TCPURGENT : 0;
		bp->b_len = i;
		bp->b_device = s->so_enpid;
		to_enp(enp->h_tonet, bp);

		u.u_base += i;
		u.u_count -= i;
		sent += i;
	}
	return (sent);
}


/*
 *	The bulk of the work, and all of the socket-related 
 *	operations, takes place as a result of ioctl calls.
 */

so_ioctl(dev, cmd, arg, fflag)
{
	register SOCKET	*s, *s2;
	register CHAN	*cp;
	register int	i;

	int		val;
	char		toname[SOCKNAMESIZE];
	short		NEEDCON, SOCKTYP, SOCKFAM;

	userparm	pblock, *pb;

	struct altblock {	/* alternate arg format used by ENP for RESV */
		long ab_af;
		long ab_type;
		long ab_proto;
	} altblock;

	s = &sock_table[(short) minor(dev)];

	if ((s->so_state & SS_OPEN) == 0) {
		u.u_error = EBADF;
		return;
	}

	/*	Set some "used a lot" test variables	*/

	NEEDCON = s->so_proto->pr_flags & PR_CONNREQUIRED;
	SOCKTYP = s->so_proto->pr_type;
	SOCKFAM = s->so_proto->pr_family;

	u.u_fmode = fflag;

	switch (cmd) {
	default:
		u.u_error = EOPNOTSUPP;
		break;

	case SIOCRESV:
		if (user_to_kern(arg, &altblock, sizeof(altblock))) {
			u.u_error = EFAULT;
			return;
		}
		if (so_resv(s, &altblock)) {
			s->so_pgrp = u.u_procp->p_pgrp;
			if ((fflag & FREAD) == 0)
				s->so_state |= SS_NOREAD;
			if ((fflag & FWRITE) == 0)
				s->so_state |= SS_NOWRITE;
		}
		break;

	case SIOCBIND:		/* bind a name to a socket */
		if (s->so_state & SS_BOUND) { 
			u.u_error = EINVAL;
			return;
		}
		if (user_to_kern(arg, &pblock, sizeof(pblock))) {
			u.u_error = EFAULT;
			return;
		}
		s->so_nmlen = min(pblock.p_len, SOCKNAMESIZE);
		if (user_to_kern(pblock.p_where, s->so_name, s->so_nmlen)) {
			u.u_error = EFAULT;
			return;
		}
		if (so_bind(s))
			s->so_state |= SS_BOUND;
		break;

	case SIOCGNAM:		/* get names associated with a socket */
		if (user_to_kern(arg, &pblock, sizeof(pblock))) {
			u.u_error = EFAULT;
			return;
		}
		so_gname(s, &pblock);
		break;

	case SIOCCONN:		/* connect to a remote socket */
		if (s->so_state & (SS_CONNECTED | SS_CONNECTING)) { 
			u.u_error = EISCONN;
			return;
		}
		if ((s->so_state & SS_BOUND) == 0) {
			if ((so_bind(s)) == 0)
				return;
			s->so_state |= SS_BOUND;
		}
		if (user_to_kern(arg, &pblock, sizeof(pblock))) {
			u.u_error = EFAULT;
			return;
		}
		i = min(pblock.p_len, SOCKNAMESIZE);
		if (user_to_kern(pblock.p_where, toname, i)) {
			u.u_error = EFAULT;
			return;
		}
		(*s->so_proto->pr_connect)(s, toname, 0, (fflag & FNDELAY));
		break;
	
	case SIOCACPT:
		if (SOCKTYP != SOCK_STREAM) {
			u.u_error = EOPNOTSUPP;
			return;
		}
		if ((s->so_state & SS_LISTEN) == 0) {
			u.u_error = EINVAL;
			return;
		}
		if ((arg = device(arg)) < 0) {
			u.u_error = EINVAL;
			return;
		}

		s2 = &sock_table[(short) minor(arg)];

		if ((s2->so_state & SS_OPEN) == 0) { 
			u.u_error = EBADF;
			return;
		}
		if ((s->so_state & SS_CONNECTING) == 0) {
			if ((s->so_state & SS_RESV) == 0) {
				if ((s->so_enpid = chan_alloc(s)) < 0) {
					u.u_error = ENOBUFS;
					return;
				}
				s->so_state |= SS_RESV;
			}
			(*s->so_proto->pr_connect)(s, 0, SS_LISTEN, 0);
		}
		if ((u.u_error = wait_for(s, (fflag & FNDELAY))) == 0) {
			*s2 = *s;
			s->so_state &= ~(SS_CONNECTED | SS_RESV);
		}
		break;

	case SIOCLISN:
		if (SOCKTYP != SOCK_STREAM) {
			u.u_error = EOPNOTSUPP;
			return;
		}
		if ((s->so_state & SS_BOUND) == 0) {
			u.u_error = EINVAL;
			return;
		}
		if (s->so_lim_listen == 0) { 
			if (arg < 0)
				arg = 1;
			s->so_lim_listen = min(arg, SOMAXCONN);
		}
		if ((s->so_state & SS_CONNECTING) == 0)
			(*s->so_proto->pr_connect)(s, 0, SS_LISTEN, 0);
		s->so_state |= SS_LISTEN;
		break;

	case SIOCSEND:
		if ((s->so_state & SS_BOUND) == 0) {
			u.u_error = EINVAL;
			return;
		}
		if ((NEEDCON) && ((s->so_state & SS_CONNECTED) == 0)) {
			u.u_error = EIO;
			return;
		}
		if (user_to_kern(arg, &pblock, sizeof (pblock))) {
			u.u_error = EFAULT;
			return;
		}
		pb = &pblock;
		u.u_base = pb->p_where;
		u.u_count = pb->p_len;
		i = sosend(s, pb->p_len2 & MSG_OOB);
		if (i > 0)
			u.u_rval1 = i;	/* return amount written */
		break;

	case SIOCSENDTO:
		if (s->so_state & SS_CONNECTED) {
			u.u_error = EISCONN;
			return;
		}
		if (user_to_kern(arg, &pblock, sizeof (pblock))) {
			u.u_error = EFAULT;
			return;
		}
		pb = &pblock;
		u.u_base = pb->p_where;
		u.u_count = pb->p_len;
		i = min(pb->p_len3, SOCKNAMESIZE);
		if (user_to_kern(pb->p_where3, toname, i)) {
			u.u_error = EFAULT;
			return;
		}
		i = sendit(s, toname, pb->p_len2 & MSG_OOB);
		if (i > 0)
			u.u_rval1 = i;	/* return amount written */
		break;

	case SIOCRECV:
		if ((s->so_state & SS_BOUND) == 0) {
			u.u_error = EINVAL;
			return;
		}
		if ((NEEDCON) && ((s->so_state & SS_CONNECTED) == 0)) {
			u.u_error = EIO;
			return;
		}
		if (user_to_kern(arg, &pblock, sizeof(pblock))) {
			u.u_error = EFAULT;
			return;
		}
		pb = &pblock;
		u.u_base = pb->p_where;
		u.u_count = pb->p_len;
		i = sorecv(s, pb->p_len2 & MSG_OOB);
		if (i > 0)
			u.u_rval1 = i;	/* return amount read */
		break;

	case SIOCRECVFROM:
		if ((s->so_state & SS_BOUND) == 0) {
			u.u_error = EINVAL;
			return;
		}
		if (s->so_state & SS_CONNECTED) {
			u.u_error = EISCONN;
			return;
		}
		if (user_to_kern(arg, &pblock, sizeof(pblock))) {
			u.u_error = EFAULT;
			return;
		}
		pb = &pblock;
		u.u_base = pb->p_where;
		u.u_count = pb->p_len;
		i = recvit(s, pb->p_where3, pb->p_len3, pb->p_len2 & MSG_OOB);
		if (i > 0)
			u.u_rval1 = i;	/* return amount read */
		break;

	case SIOCATMARK:
		i = s->so_state & SS_ATMARK ? 1 : 0;
		suword(arg, i);
		break;

	case SIOCSPGRP:
		i = fuword(arg);
		if (i != -1) {
			s->so_pgrp = i;
			if (s->so_state & SS_RESV) {
				cp = &data_chan[(short) s->so_enpid];
				cp->c_pgrp = i;
			}
		}
		else
			u.u_error = EFAULT;
		break;

	case SIOCGPGRP:
		suword(arg, s->so_pgrp);
		break;

	case SIOCSOPT:		/* set options */
		if (user_to_kern(arg, &pblock, sizeof(pblock))) {
			u.u_error = EFAULT;
			return;
		}
		s->so_options |= pblock.p_len2;
		if (s->so_state & SS_RESV) {
			cp = &data_chan[(short) s->so_enpid];
			cp->c_options = htons(ntohs(cp->c_options)|pblock.p_len2);
		}
		break;

	case SIOCGOPT:		/* get options */
		val = s->so_options;
		if (s->so_state & SS_RESV) {
			cp = &data_chan[(short) s->so_enpid];
			val |= ntohs(cp->c_options);
		}
		if (user_to_kern(arg, &pblock, sizeof(pblock))) {
			u.u_error = EFAULT;
			return;
		}
		suword(pblock.p_where, val);
		suword(pblock.p_len, sizeof(val));
		break;

	case SIOCSHUT:
		arg++;
		cp = &data_chan[(short) s->so_enpid];
		if (arg & 1) {
			s->so_state |= SS_NOREAD;
			cp->c_state = htons(ntohs(cp->c_state)|SS_FLUSHING);
		}
		if (arg & 2) {
			s->so_state |= SS_NOWRITE;
			so_close(dev);
		}
		break;
	}
}

#ifdef SELECT
so_select( dev, flag )
{
	register SOCKET *s;
	register CHAN   *cp;
	register struct proc *p;
	int x;
	extern	selwait;

	x = splnet();

	s = &sock_tab[ minor( dev ) ];
	cp = &data_chan[ s->so_enpid ];

	switch( flag ) {
	case FREAD:
		if ( fir ( &cp->c_tohost ) ) {
			splx( x );
			return ( 1 );
		}
		if ( (p = (struct proc *)cp->c_selproc)
					&& (p->p_wchan == (caddr_t) &selwait) )
			cp->c_state |= SS_SELCOLL;
		else
			cp->c_selproc = (caddr_t) u.u_procp;
		break;

	case FWRITE:
		if ( s->so_proto->pr_flags&PR_CONNREQUIRED )
			return( s->so_state&SS_CONNECTED );
		else {
			splx( x );
			return( 1 );
		}
		break;
	}

	splx( x );
	return( 0 );
}
#endif


typedef
struct PBLOCK {
	long	p_domain;		/* AF */
	long	p_type;			/* STREAM, DGRAM, etc */
	long	p_proto;		/* TCP, UDP, etc */
} PBLOCK;

static
so_resv(s, pb)
register SOCKET	*s;
register PBLOCK *pb;
{
	register PROTO	*pr;

	pr = (pb->p_proto) ? (PROTO *) pffindproto(pb->p_domain, pb->p_proto) :
			     (PROTO *) pffindtype(pb->p_domain, pb->p_type);

	if ((pr == 0) || (pb->p_type && (pr->pr_type != pb->p_type))) {
		u.u_error = ESOCKTNOSUPPORT;
		return (0);
	}
	else {
		s->so_proto = pr;
		if ((s->so_enpid = chan_alloc(s)) == -1) {
			u.u_error = ENOBUFS;
			return (0);
		}
		s->so_state |= SS_RESV;
		return (1);
	}
}


static
so_bind(s)
register SOCKET	*s;
{
	register PROTO	*pr = s->so_proto;

	if ((u.u_error = (*pr->pr_nmck)(s)) == 0) {
		if ((u.u_error = namecheck(pr, s->so_name, s->so_nmlen)) == 0)
			u.u_error = (*pr->pr_bind)(s);
	}

	return (u.u_error == 0);
}


static
so_gname(s, pb)
register SOCKET	*s;
register userparm *pb;
{
	register int	size;

	char		name[SOCKNAMESIZE];

	if ((size = (*s->so_proto->pr_gname)(s, name)) != 0) {
		if (pb->p_where)
			kern_to_user(name, pb->p_where, size);
		if (pb->p_len)
			suword(pb->p_len, size);
		if (pb->p_where2)
			kern_to_user(name+size, pb->p_where2, size);
		if (pb->p_len2)
			suword(pb->p_len2, size);
	}
}


wait_for(s, dontwait)
register SOCKET	*s;
{
	register BFR	*bp;
	register OPN	*op;
	register CHAN	*cp;
	register int	x;

	op = s->so_o_opn;
	bp = s->so_o_bfr;

	x = splnet();
	while (op->o_status == -1) {
		if (dontwait) {
			splx(x);
			return (EWOULDBLOCK);
		}
		sleep(bp, PENP);
	}
	splx(x);
	x = ntohs(op->o_status);

	s->so_state &= ~SS_CONNECTING;
	s->so_o_bfr = 0;
	s->so_o_opn = 0;
	fre_buf(bp);

	if (x == 0)
		s->so_state |= SS_CONNECTED;

#ifdef SELECT
	cp = &data_chan[(short) s->so_enpid];
	if (cp->c_selproc) {
		selwakeup(cp->c_selproc, ntohs(cp->c_state)&SS_SELCOLL);
		cp->c_selproc = 0;
		cp->c_state = htons(ntohs(cp->c_state) & ~SS_SELCOLL);
	}
#endif
	return (x);
}



chan_alloc(s)
register SOCKET	*s;
{
	register PROTO	*pr = s->so_proto;
	register CHAN	*cp;
	register short	i;

	for (i = pr->pr_next; i < pr->pr_end; i++)
	if (data_chan[i].c_state == SS_AVAIL)
		goto found;

	for (i = pr->pr_channel; i <= pr->pr_next; i++)
	if (data_chan[i].c_state == SS_AVAIL)
		goto found;

	printf("chan_alloc:  No Available Channels\n");
	return (-1);

found:
	chan_inuse++;
	cp = &data_chan[i];
	cp->c_state = htons(SS_ALLOC);
	enp_assign(cp->c_selproc, 0);
	cp->c_pgrp = s->so_pgrp;
	cp->c_options = htons(s->so_options);

	pr->pr_next = i + 1;
	if (pr->pr_next == pr->pr_end)
		pr->pr_next = pr->pr_channel;

	return (i);
}


#define	ADDDOMAIN(x)	{ \
	extern struct domain x/**/domain; \
	x/**/domain.dom_next = domains; \
	domains = &x/**/domain; \
}


domaininit()
{
#ifndef lint
	ADDDOMAIN(inet);
	ADDDOMAIN(eth);
#endif
}


pffindtype(family, type)
int	family, type;
{
	register struct domain *dp;
	register struct protosw *pr;

	for (dp = domains; dp; dp = dp->dom_next)
		if (dp->dom_family == family)
			goto found;
	return (0);
found:
	for (pr = dp->domps; pr < dp->domps_END; pr++)
		if (pr->pr_type && pr->pr_type == type)
			return ((int)pr);
	return (0);
}

pffindproto(family, protocol)
int	family, protocol;
{
	register struct domain *dp;
	register struct protosw *pr;

	if (family == 0)
		return (0);
	for (dp = domains; dp; dp = dp->dom_next)
		if (dp->dom_family == family)
			goto found;
	return (0);
found:
	for (pr = dp->domps; pr < dp->domps_END; pr++)
		if (pr->pr_protocol == protocol)
			return ((int)pr);
	return (0);
}

namecheck(proto, nm, nmlen)
register PROTO	*proto;
caddr_t		nm;
{
	register SOCKET	*s;
	register	res;

	for (s = &sock_table[0]; s < &sock_table[NSOCKETS]; s++) {
		if ((s->so_proto == proto) && (s->so_state&SS_BOUND)) {
			res = (*proto->pr_cmp) (s->so_name, nm, nmlen,
				s->so_options & SO_REUSEADDR);
			if (res)
				return (res);
		}
	}
	return (0);
}
