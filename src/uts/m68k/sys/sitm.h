/************************************************************************
*
*	File:		sitm.h (sock it to me)
*	Function:	specific definitions for sitm.c
*
*************************************************************************/

#define SCK_NUM(s)	((struct socket *)(s) - sock_table)
#define SOCK_ID		6969				/* module id number */
#define MAX_TCP_DATA	1460				/* max tcpip data   */
#define TCPURGENT	0x40				/* urgent? */

/*
 *	Socket Disconnection Detection Definitions
 */
#define	SDD_TIMEOUT	3000		/* timeout value for scktimer() */

/*
 *	SITM header structure and defines
 */
typedef
struct sitm_hdr {
	short	type;			/* type of packet */
	short	spare;			/* saved for later */
} SITM;

/* values for type */
#define	SITM_DATA	0		/* data packet */
#define SITM_CTL	1		/* Control packet */
#define SITM_ACTIVE	2		/* Activity response requested */

/*
 *	CONN structure - used to retry connects that fail
 *			 due to no enp buffers
 */
typedef
struct conn {
	char	*to;			/* pointer to toname block or zero */
	char	toname[8];
	unsigned	type;		/* connection type */
	unsigned	flags;		/* wait/no wait flags */
} CONN;
/*
 *	TI characteristics of the SOCK IT TO ME module
 */
#define TSDU_SIZE	INFPSZ
#define ETSDU_SIZE	0
#define TIDU_SIZE	MAX_TCP_DATA	
#define CDATA_SIZE	0
#define DDATA_SIZE	0
#define ADDR_SIZE	sizeof(struct sockaddr_in)
#define OPT_SIZE	0

/*
 *	Parameter block
 */
typedef
struct PBLOCK {
	long	p_domain;		/* AF */
	long	p_type;			/* STREAM, DGRAM, etc */
	long	p_proto;		/* TCP, UDP, etc */
} PBLOCK;

/*
 *
 *	Sitm address format
 *		xxxx yy zzzzzz
 *
 *		xxxx = port id (normally 022b)
 *		yy   = net id
 *		zzzzzz = unique INET addr
 *
 *		Class a addressing used but not required
 */
#define	SCK_ADDR_SIZE	6		/* address used by tli users */
typedef
struct local_addr {
	short	port;			/* internet port		*/
	long	addr;			/* internet unique address	*/
} LOCAL_ADDR;


