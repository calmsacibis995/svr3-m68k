/*
 *	Copyright (c) 1984, Communication Machinery Corporation
 */

/*
 *	This file contains material and information which is
 *	proprietary to Communication Machinery Corporation, and
 *	which may not be divulged without the express written
 *	permission of Communication Machinery Corporation.
 */

/*
 *	This file defines the data structures used to communicate
 *	between an ENP and a Unix host.
 *
 *	This file is needed by a device-driver type interface to
 *	an ENP.  It presumes data structure formats as defined
 *	in <CMC/user_socket.h>.
 */

#define CHANSIZE	4
#define NCHANS		32
#define NSOCKETS 	32
#define SOMAXCONN 	5
#define SOCKNAMESIZE	32

/*
 *	Formats of shared data structures
 */

/*
 * per connection data flow channels 
 */

typedef struct sring
{
	char	s_rdidx;
	char	s_wrtidx;
} SRING;

/*
 *	macros to manipulate "shadow" rings.
 *
 *	There are some assumptions built in here regarding the
 *	size of ring involved.  Invoking sringput if sringfull is true
 *	will almost certainly result in disaster.
 */

# define sringinit(key) ( data_chan[ (key) ].c_sring.s_rdidx = data_chan[ (key) ].c_sring.s_wrtidx = 0 )
# define sringempty(sp) ( (sp)->s_wrtidx == (sp)->s_rdidx )
# define sringfull(sp) ( (((sp)->s_wrtidx+1) & (CHANSIZE-1)) == ((sp)->s_rdidx&(CHANSIZE-1)) )
# define sringput(sp)  ( (sp)->s_wrtidx = ((sp)->s_wrtidx + 1) & (CHANSIZE-1) )
# define sringget(sp)  ( (sp)->s_rdidx =  ((sp)->s_rdidx + 1) & (CHANSIZE-1) )

typedef struct chan
{
	short	c_state;		/* state of channel */
	short	c_pgrp;			/* process group for urgent */
	short	c_options;		/* socket level options */
	short	c_inflight;		/* number in use */
	BFR	*c_opbfr;		/* bfr waiting open to complete */
	BFR	*c_clbfr;		/* bfr waiting close to complete */
#ifndef lint
	RING/**/CHANSIZE	c_tohost;	/* bfrs for the host */
#else
	RING4			c_tohost;
#endif
	SRING	c_sring;		/* shadow ring */
	short	c_pad;
	union {
		caddr_t	c_sel;		/* procp of selecting process */
		caddr_t c_str;		/* stream socket pointer      */
	} C_un;
#define c_selproc	C_un.c_sel
#define c_strsock	C_un.c_str
	char 	c_name[ SOCKNAMESIZE ];	/* current name info */
}CHAN;

/*
 * bus rings
 */

typedef struct ENPDATA
{
	RING8	h_open;			/* open requests to enp */
	RING16	h_tonet;		/* data,stat,cls packets */
	RING16	h_netcomplete;		/* finished data,stat,cls packets */
	RING32  e_tohost;		/* Everything from ENP */
	RING32	e_hostcomplete;		/* Completed back from host */
	RING16	e_wakeaddr;		/* addrs to wake up on intr */
	RING8	e_urg;			/* pgrps for urgent signals */
} ENPDATA;


typedef
struct socket
{
	short		so_options;
	short		so_state;	/* See definitions below	*/
	short		so_enpid;	/* Channel Id - to/from ENP	*/
	short		so_lim_listen;
	short		so_pgrp;	/* Unix process group		*/
	short		so_nmlen;	/* Num bytes in so_name		*/
	char		so_name[SOCKNAMESIZE];  /* User supplied name	*/
	PROTO		*so_proto;	/* releated protocol		*/
	BFR		*so_o_bfr;	/* If open in prog- BFR address	*/
	struct opn	*so_o_opn;	/* and addr of params in BFR	*/
	caddr_t		so_strq;	/* stream queue pointer         */
	short		so_link;	/* link state flag for disco    */
	short		so_tli;		/* Transport Layer state	*/
} SOCKET;


/*
 *	Values for so_state
 */

# define	SS_AVAIL	0
# define	SS_OPEN		1
# define	SS_CONNECTED	2
# define	SS_LISTEN	4
# define	SS_CONNECTING	8
# define	SS_NOREAD	0x10
# define	SS_NOWRITE	0x20
# define	SS_FLUSHING	0x40
# define	SS_BROKEN	0x80
# define	SS_RESV		0x100
# define	SS_ATMARK	0x200
# define	SS_BOUND	0x400
# define	SS_CLOSEING	0x800
# define	SS_SELCOLL	0x1000
# define	SS_ALLOC	0x2000

/*
 * Definitions for so_link - used by Disconnect detection
 */
#define SDD_DEAD	0		/* not stream or disconnected */
#define	SDD_ACTIVE	1		/* activity on socket/stream */
#define SDD_INACTIVE	2		/* reset value after poll */
#define SDD_PEND	3		/* status not yet determined */

/*
 * Definitions related to sockets: types, address families, options.
 */

/*
 * Types
 */

#define	SOCK_STREAM	1		/* stream socket */
#define	SOCK_DGRAM	2		/* datagram socket */
#define	SOCK_RAW	3		/* raw-protocol interface */
#define	SOCK_RDM	4		/* reliably-delivered message */
#define	SOCK_SEQPACKET	5		/* sequenced packet stream */

/*
 * Option flags per-socket.
 */
#define	SO_DEBUG	0x01		/* turn on debugging info recording */
#define	SO_ACCEPTCONN	0x02		/* socket has had listen() */
#define	SO_REUSEADDR	0x04		/* allow local address reuse */
#define	SO_KEEPALIVE	0x08		/* keep connections alive */
#define	SO_DONTROUTE	0x10		/* just use interface addresses */
				/* 0x20 was SO_NEWFDONCONN */
#define	SO_USELOOPBACK	0x40		/* bypass hardware when possible */
#define	SO_LINGER	0x80		/* linger on close if data present */
#define	SO_DONTLINGER	(~SO_LINGER)	/* ~SO_LINGER */


/*
 * Address families.
 */
#define	AF_UNSPEC	0		/* unspecified */
#define	AF_UNIX		1		/* local to host (pipes, portals) */
#define	AF_INET		2		/* internetwork: UDP, TCP, etc. */
#define	AF_IMPLINK	3		/* arpanet imp addresses */
#define	AF_PUP		4		/* pup protocols: e.g. BSP */
#define	AF_CHAOS	5		/* mit CHAOS protocols */
#define	AF_NS		6		/* XEROX NS protocols */
#define	AF_NBS		7		/* nbs protocols */
#define	AF_ECMA		8		/* european computer manufacturers */
#define	AF_DATAKIT	9		/* datakit protocols */
#define	AF_CCITT	10		/* CCITT protocols, X.25 etc */
#define	AF_SNA		11		/* IBM SNA */
#define AF_ETHER	12		/* direct Ethernet access */
#define AF_ETHERLINK	12		/* direct Ethernet access */

#define	AF_MAX		13

/*
 * Protocol families, same as address families for now.
 */
#define	PF_UNSPEC	AF_UNSPEC
#define	PF_UNIX		AF_UNIX
#define	PF_INET		AF_INET
#define	PF_IMPLINK	AF_IMPLINK
#define	PF_PUP		AF_PUP
#define	PF_CHAOS	AF_CHAOS
#define	PF_NS		AF_NS
#define	PF_NBS		AF_NBS
#define	PF_ECMA		AF_ECMA
#define	PF_DATAKIT	AF_DATAKIT
#define	PF_CCITT	AF_CCITT
#define	PF_SNA		AF_SNA
#define PF_ETHER	AF_ETHER
#define PF_ETHERLINK	AF_ETHERLINK

#define	PF_MAX		AF_MAX

#define	MSG_OOB		0x1		/* process out-of-band data */
#define	MSG_PEEK	0x2		/* peek at incoming message */
#define	MSG_DONTROUTE	0x4		/* send without using routing tables */

/*
 *	Unix priority
 */

# define PENP PZERO+1

/*
 *	Variables declared elsewhere
 */

extern SOCKET sock_table[];	/* The actual sockets */

/*
 * open a connection data structure 
 */

typedef struct opn
{
	short	o_status;	/* how it came out */
	short	o_pgrp;		/* proc group for urgent sigs */
	short	o_type;		/* listen/open what */
	short	o_pad;		/* for alignment	*/
	int	o_id;		/* returned id */
	char	o_name[ SOCKNAMESIZE ];
	char	o_parm[ SOCKNAMESIZE ];
}OPN;

/*
 *	Protocol_Initialization_Block
 *
 *	Used by host to tell ENP which INTERFACE protocols to init.
 *
 *	The host will set "protosw"->pr_channel & pr_available and
 *	the ENP will set the p_start & p_end values in the interface
 *	layer's PROTOCOL structure (proto.h) using these values.
 */

#define MAXPNAME	12

typedef struct	pib {
	short	pib_family;		/* Host protosw->pr_family	*/
	short	pib_type;		/* Host protosw->pr_type	*/
	int	pib_start;		/* ENP protocol->p_start value	*/
	int	pib_num;		/* ENP protocol->p_end		*/
	char	pib_name[MAXPNAME];	/* ENP protocol->p_name		*/
} PIB;
