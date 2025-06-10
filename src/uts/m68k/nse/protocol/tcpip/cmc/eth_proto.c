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
 *	Defines for types, header/data offsets in protosw
 */

# define MAX_ETH_HDR	0		/* Where data starts	*/
# define MAX_ETH_DATA	1514		/* Max. size of it	*/


/*
 *	External Globals we use
 */

extern
SHARED_RINGS	*enp;

extern CHAN	*data_chan;


extern	eth_cmp(), ethnmchk(), ethgname();
extern	ethbind(), ethconnect(), ethclose(), ethsend(), ethrecv();


/*
 *	The protocol switch table ...
 */

struct protosw ethsw[] = {
	{ SOCK_RAW, PF_ETHER, ETHPROTO_RAW, PR_ATOMIC|PR_ADDR,
	  eth_cmp, ethnmchk, ethgname,
	  ethbind, ethconnect, ethclose, ethsend, ethrecv,
	  0, 0, 0, MAX_ETH_HDR, MAX_ETH_DATA }
};


/*
 *	The Ethernet domain ...
 */

struct domain ethdomain = {
	AF_ETHER, "ethernet", ethsw, &ethsw[sizeof(ethsw)/sizeof(ethsw[0])]
};


/*
 *	"Generic" Ethernet routines
 */

en_addr	MYNODEPARAM;

static
en_addr	zero_addr = {0};


static
eth_cmp(inp1, inp2, len, reuse)
register sockaddr_eth *inp1, *inp2;
{
	if (inp1->sen_type && (inp1->sen_type == inp2->sen_type))
		return (EADDRINUSE);

	return (0);
}


ethgname(s, place)
register SOCKET	*s;
{
	register sockaddr_eth *csen;

	if (s->so_state & SS_BOUND)
		enp_move(s->so_name, place, sizeof(sockaddr_eth));
	else
		enp_zero(place, sizeof(sockaddr_eth));

	if (s->so_state & SS_CONNECTED) {
		register sockaddr_eth *ap;

		ap = (sockaddr_eth *) place + 1;
		ap->sen_family = s->so_proto->pr_family;
		csen = (sockaddr_eth *) data_chan[(short) s->so_enpid].c_name;
		ap->sen_type = ntohs(csen->sen_type);
		memcpy(&ap->sen_addr, &csen->sen_addr, sizeof(en_addr));
	}
	else
		enp_zero((sockaddr_eth *) place + 1, sizeof(sockaddr_eth));

	return (sizeof(sockaddr_eth));
}


ethnmchk(s)
register SOCKET	*s;
{
	register sockaddr_eth *sep;
	register unsigned short type;

	extern en_addr MYNODEPARAM;

	sep = (sockaddr_eth *) s->so_name;

	if ((type = sep->sen_type) == 0)
		return (EADDRNOTAVAIL);

	if (memcmp(&sep->sen_addr, &zero_addr, sizeof(zero_addr)) == 0)
		memcpy(&sep->sen_addr, &MYNODEPARAM, sizeof(en_addr));

	return (0);
}
