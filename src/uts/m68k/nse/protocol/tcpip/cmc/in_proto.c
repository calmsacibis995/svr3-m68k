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
 *	The following defines for MAXs are temporary - these values
 *	will be provided by the protocol running in the ENP at init
 *	time. (soon)
 */


/*
 *	TCP Specific
 */

#define	MAX_TCP_HDR	54		/* LOCAL + INHDR + TCPHDR	*/
#define MAX_TCP_DATA	1460		/* Max Ether - Max TCP/IP Hdr	*/

extern	tcpbind(), tcpconnect(), tcpclose(), tcpsend(), tcprecv();


/*
 *	UDP Specific
 */

#define	MAX_UDP_HDR	42		/* LOCAL + INHDR + UDHDR	*/
#define	MAX_UDP_DATA	1472		/* Max Ether - Max UDP/IP Hdr	*/

extern	udpbind(), udpconnect(), udpclose(), udpsend(), udprecv();


/*
 *	RIP Specific (Raw IP)
 */

#define	MAX_RIP_HDR	34		/* LOCAL + INHDR		*/
#define	MAX_RIP_DATA	1480		/* Max Ether - Max IP hdr	*/

extern	ripnmchk();
extern	ripbind(), ripconnect(), ripclose(), ripsend(), riprecv();


/*
 *	Generic
 */

extern	ip_cmp();
extern	ipgname(), ipnmchk();


/*	The protocol switch		*/

struct protosw	inetsw[] = {
	{ SOCK_STREAM, PF_INET, IPPROTO_TCP, PR_CONNREQUIRED,
	  ip_cmp, ipnmchk, ipgname,
	  tcpbind, tcpconnect, tcpclose, tcpsend, tcprecv,
	  0, 0, 0, MAX_TCP_HDR, MAX_TCP_DATA },

	{ SOCK_DGRAM, PF_INET, IPPROTO_UDP, PR_ATOMIC,
	  ip_cmp, ipnmchk, ipgname,
	  udpbind, udpconnect, udpclose, udpsend, udprecv,
	  0, 0, 0, MAX_UDP_HDR, MAX_UDP_DATA },

	{ SOCK_RAW, PF_INET, IPPROTO_RAW, PR_ATOMIC,
	  ip_cmp, ripnmchk, ipgname,
	  ripbind, ripconnect, ripclose, ripsend, riprecv,
	  0, 0, 0, MAX_RIP_HDR, MAX_RIP_DATA }
};


/*	The Protocol Domain	*/

struct domain	inetdomain = {
	AF_INET, "internet", inetsw, &inetsw[sizeof(inetsw)/sizeof(inetsw[0])]
};


/*
 *	Generic INET name routines
 */

static
ip_cmp(inp1, inp2, len, reuse)
register sockaddr_in *inp1, *inp2;
{
	if (inp1->sin_port && ((inp1->sin_port == inp2->sin_port) && !reuse))
		return (EADDRINUSE);
	return (0);
}


extern
SHARED_RINGS	*enp;

extern CHAN	*data_chan;
HOST	MYHOSTPARAM;

static
sockaddr_in	sin = {AF_INET};


ipgname(s, place)
register SOCKET	*s;
{
	register sockaddr_in *csin;

	if (s->so_state & SS_BOUND)
		enp_move(s->so_name, place, sizeof(sockaddr_in));
	else
		enp_zero(place, sizeof(sockaddr_in));

	if (s->so_state & SS_CONNECTED) {
		register sockaddr_in *ap;

		ap = (sockaddr_in *) place + 1;
		ap->sin_family = s->so_proto->pr_family;
		csin = (sockaddr_in *) data_chan[(short) s->so_enpid].c_name;
		ap->sin_port = ntohs(csin->sin_port);
		ap->sin_addr.s_addr = ntohl(enp_rval(csin->sin_addr.s_addr));
	}
	else
		enp_zero((sockaddr_in *) place + 1, sizeof(sockaddr_in));

	return (sizeof(sockaddr_in));
}


ipnmchk(s)
register SOCKET	*s;
{
	register sockaddr_in *inp;
	register short try, inc, current;
	static	 short curr_priv = IPPORT_RESERVED,
		curr_unpriv = IPPORT_RESERVED;

# define bounded(x) ((x>IPPORT_RESERVED/2)&&(x<IPPORT_RESERVED*2))

	inp = (sockaddr_in *) s->so_name;
	if (inp->sin_addr.s_addr == 0)
		inp->sin_addr.s_addr = MYHOSTPARAM;

	if (inp->sin_port == 0) { 	/* we'll pick one */
		if ( u.u_uid < 20 ) {
			current = curr_priv;
			inc = -1;
		}
		else {
			current = curr_unpriv;
			inc = 1;
		}
		for (try = current+inc; bounded(try); try += inc) { 
			sin.sin_port = try;
			if (namecheck(s->so_proto, &sin, sizeof (sin)) == 0)
				goto gotone;
		}
		for (try = IPPORT_RESERVED+inc; try != current; try += inc) { 
			sin.sin_port = try;
			if (namecheck(s->so_proto, &sin, sizeof (sin)) == 0)
				goto gotone;
		}
		/* if we get here, failure */
		return (EADDRNOTAVAIL);

gotone:
		inp->sin_port = try;
		s->so_nmlen = sizeof(sin);
		if ( u.u_uid < 20 )
			curr_priv = try;
		else	curr_unpriv = try;
	}

	if ((inp->sin_port < IPPORT_RESERVED) && (u.u_uid >= 20))
		return (EACCES);
	return (0);
}
