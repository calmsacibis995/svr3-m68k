/*
 *	Copyright (c) 1984 by Communication Machinery Corporation
 */

/*
 *	This file contains material and information which is proprietary
 *	to Communication Machinery Corporation.  Duplication or disclosure
 *	of this information is prohibited without the express written
 *	authorization of Communication Machinery Corporation.
 */

/*
 *	Ethernet-style addresses
 */

typedef
struct en_addr {
	char	ena_addr[6];
} en_addr;

typedef
struct sockaddr_eth {
	short	sen_family;
	u_short	sen_type;		/* Ethernet type code */
	en_addr	sen_addr;
	char	sen_zero[4];
} sockaddr_eth;

# define ETHTYPE_ANY 0xFFFF

/*
 *	Definitions for getsockopt and setsockopt
 *
 *	Usage is:
 *		unsigned short arg;
 *		setsockopt (s, SOL_ETHER, EIOCSETMODE, &arg, sizeof (arg));
 */

# define SOL_ETHER 0

# define EIOCSETMODE (128)
# define EIOCGETMODE (129)
# define EIOCGETSTAT (130)
# define EIOCCLRSTAT (131)
# define EIOCGETSTATCLR (132)
# define EIOCSETIME (133)
# define EIOCSETTIME EIOCSETIME
# define EIOCSETBUF (134)
# define EIOCGETBUF (135)

/*
 *	Bit definitions for SETMODE
 *	(may be OR'd together)
 */

# define PROMISCUOUS 0x8000
# define DOSOURCE    0x4000
# define USEADDR     0x2000	/* use supplied src, not node address */
# define LOOPINTERN  0x0040
# define DONTRETRY   0x0020
# define FRCECOLLIDE 0x0010
# define NOXMITCRC   0x0008
# define LOOPBACK    0x0004
# define NOXMITTER   0x0002
# define NORECVER    0x0001

/*
 *	Bits returned by GETMODE
 *		Many of these indicate potential errors, but there is 
 *		no way to correlate the errors with any particular Unix
 *		read or write.
 */

# define ANYERROR	0x8000
# define BABBLE		0x4000
# define COLLISION	0x2000
# define MISS		0x1000
# define MEMERR		0x0800
# define RCVINT		0x0400
# define XMITINT	0x0200
# define INITDONE	0x0100
# define INTPENDING	0x0080
# define INTENABLE	0x0040
# define RCVRON		0x0020
# define XMITON		0x0010
# define DEMANDXMIT	0x0008
# define STOPPED	0x0004
# define STARTED	0x0002
# define INITIALIZE	0x0001

/*
 *	ioctl structures for statistics/mode
 */

typedef
struct eth_io_stat
{
	unsigned long e_xmit_successful;	/* Successful transmissions */
	unsigned long e_mult_retry;		/* multiple retries on xmit */
	unsigned long e_one_retry;		/* single retries */
	unsigned long e_fail_retry;		/* too many retries */
	unsigned long e_deferrals;		/* transmission delayed due
						   to active medium */
	unsigned long e_xmit_buff_err;		/* xmit data chaining failed --
						   "can't happen" */
	unsigned long e_silo_underrun;		/* transmit data fetch failed */
	unsigned long e_late_coll;		/* collision after xmit */
	unsigned long e_lost_carrier;
	unsigned long e_babble;			/* xmit length > 1518 */
	unsigned long e_collision;
	unsigned long e_xmit_mem_err;
	unsigned long e_rcv_successful;		/* good receptions */
	unsigned long e_rcv_missed;		/* no recv buff available */
	unsigned long e_crc_err;		/* checksum failed */
	unsigned long e_frame_err;		/* crc error AND
						   data length != 0 mod 8 */
	unsigned long e_rcv_buff_err;		/* rcv data chain failure --
						   "can't happen" */
	unsigned long e_silo_overrun;		/* receive data store failed */
	unsigned long e_rcv_mem_err;
} Eth_Io_Stat;

/*
 * Bits of errno for read errors.
 */

#define ER_FRAM	0x20	/* A framing error occurred. */
#define ER_OFLO	0x10	/* A SILO overflow occured. */
#define ER_CRC	0x08	/* A CRC error was detected. */
#define ER_BUFF	0x04	/* A receive buffer error occurred. */
#define ER_MERR	0x02	/* A memory error occurred on receive. */
#define ER_MISS	0x01	/* A missed packet was reported. */

/*
 * Bits of errno for write errors.
 */
#define EX_UFLO	0x40	/* Silo underflow occurred on transmission. */
#define EX_BABL	0x20	/* A babbling transmitter error was reported. */
#define EX_LCOL	0x10	/* A late collision occurred on transmission. */
#define EX_CAR	0x08	/* Carrier was lost during transmission. */
#define EX_RTRY	0x04	/* Retry failure. */
#define EX_MERR	0x02	/* A memory error occured on transmit. */
#define EX_CERR	0x01	/* A collision error was reported. */

#if !defined(vax)
/*
 * Macros for number representation conversion.
 */
#define	ntohl(x)	(x)
#define	ntohs(x)	(x)
#define	htonl(x)	(x)
#define	htons(x)	(x)
#endif
