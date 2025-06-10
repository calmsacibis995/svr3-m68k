
/*	@(#)mvme331.h	1.1		*/

/****************************************************************************/
/*   Data Structures for intelligent communication controller mvme331/332   */ 
/****************************************************************************/


/* CONFIGURATION */

#define MAXICCS		6	/* maximum count of ICCs per system */
#define MAXPORTS	8	/* maximum count of ports per cntl */
#define MAXITEMS	42	/* maximum count of items per configuration */

#define	PRIICC		PZERO	/* sleep priority for the driver */


/* DECLARATIONS */

/* description of one intelligent communication controller (ICC) */
struct icc {
	int	c_stat[MAXPORTS];	/* status of a port */
	int	c_intrlevel;		/* interrupt level */
	int	c_intrvector;		/* interrupt vector */
	int	c_portscount;		/* count of ports */
	int	c_firsttty;		/* index to first tty struct. */
	struct CCpacket *c_pkread[MAXPORTS];  /* read packet pointer */
	struct CCpacket *c_pkwrite[MAXPORTS]; /* write packet pointer */
	struct CCpacket *c_pksndsol[MAXPORTS];/* send solicited event
						  packet pointer */
};

/* description of the cross reference between tty structure index
   and cntl / port */
struct tpref {
	int	t_cntl;			/* cntl # */
	int	t_port;			/* port  # */
};

/* structures for configuring controller or port */
struct ICCitem {
	short	item;		/* item number */
	int	value;		/* value for this item */
};

struct confbuf {
	struct ICCitem param[MAXITEMS];
	short  index;
	short  resetflg;
};
/* DEFINITIONS */

/* stat codes */
#define ICCCLEAR	0x00		/* port is cleared */
#define ICCINIT		0x01		/* port is initialized */
#define ICCREAD		0x02		/* read started */
#define ICCWRITE	0x04		/* write started */
#define ICCABORT	0x08		/* abort started */
#define ICCWRABORT	0x10		/* write aborted */
#define ICCRDSTART	0x20		/* read started */
/* SSM 05/27/86 - 1 */
#define ICCHWHAND	0x40		/* hardware handshake on */


/* ICC command codes */
#define	CMD_READ	1
#define	CMD_WRITE	2
#define	CMD_ABORT	3
#define	CMD_SNDSOLEVENT	4
#define	CMD_CFICC	5
#define	CMD_CFPORT	6
#define	CMD_CFTABLE	7
/* commands 8 - 10 are not used */
#define	CMD_STICC	11
#define	CMD_STPORT	12
#define	CMD_STTABLE	13
#define	CMD_RESET	14
#define	CMD_SNDBREAK	15
#define	CMD_RCVFLUSH	16
#define	CMD_TRANSFLUSH	17
/* commands 18 and 19 are not used */

/* solicited event definitions */
/* mask definitions */
#define BREAKEVENT	0x00010000	/* BREAK event */
#define	DCDEVENT	0x00040000	/* DCD changed */
#define	CTSEVENT	0x00020000	/* CTS changed */

/* state definitions */
#define	DCDSTAT		0x00000004	/* DCD set */
 
/* ioctl commands to command channel */
#define MICCSTAT	(('m'<<8)+11)	/* request ICC status */
#define MPORTSTAT	(('m'<<8)+12)	/* request port status */
#define MTABLESTAT	(('m'<<8)+13)	/* request table status */
/* SSM 05/27/86 - 1 */
#define MSETHWHAND	(('m'<<8)+20)	/* set hardware handshake */
#define MCLEARHWHAND	(('m'<<8)+21)	/* clear hardware handshake */
#define MGETHWHAND	(('m'<<8)+22)	/* get hardware handshake mode */

#define SIZEOFICCSTAT	48	/* length of returned status by
				   request ICC status */

#define SIZEOFPORTSTAT	37*4	/* length of returned status by
				   request port status */

#define SIZEOFTABLESTAT	256	/* length of returned status by
				   request table status */

#define SIZEOFHALTSAVE	132	/* length of halt save table */

/* SSM 05/20/86 - 1 */
#define PORTBUFLENGTH   258     /* length of PortBuffer */

/*--------------------------------------------------------------*/
/* HEADER INFORMATION FOR COMMAND CHANNEL INTERFACE */

/* CONFIGURATION */

#define MAXCNTLS	6	/* max. count of controllers */
#define PKMAX		32	/* max. count of data packets / cntl */
#define PTMAX		32	/* max. count of pointers / cntl */
#define ISRMAXRET	16	/* maximal returned packets by interrupt
				   service routine */

#define PRICCI		PZERO   /* sleep priority for this module */

#define	COMREGOFFSET	1	/* offset to baseaddress for 
				   communication register */
#define	ATTREGOFFSET	0	/* offset to baseaddress for 
				   attention register */



/* DECLARATIONS for POOL MANAGER */

/* description of the command channel per driver */
struct CCdriver {
	int	cd_stat;	/* status of command channel */
	int	cd_cntlcount;	/* count of controllers */
	int	cd_pkcount;	/* count of available packets */
	struct	CCpacket *cd_pknextfree; /* next free packet */
};

/* stat codes */
#define CDCLEAR 0x00
#define CDINIT	0x01
#define CDBUSY	0x02
#define CDERROR	0x04


/* description of the command channel per controller */
struct CCcontroller {
	int	cc_stat;	/* status of this controller */
	int	cc_ptcount;	/* count of available pointers */
	char	*cc_comreg;	/* address of communication register */
	char	*cc_attreg;	/* address of attention register */
	int	cc_intrvector;	/* interrupt vector */
	int	cc_intrlevel;	/* interrrupt level */
	struct	CCpointer *cc_cmdsep;	/* command seperator */
	struct	CCpointer *cc_statsep;	/* status seperator */
	struct	CCpointer *cc_ptnextfree;/* next free pointer */
};

/* stat codes */
#define CCCLEAR 0x00
#define CCINIT	0x01
#define CCBUSY	0x02
#define CCERROR	0x04


/* Pointer structure for command channel */
struct CCpointer {
	char 		 *pt_datapt;/* own packet data pointer */
	struct CCpointer *pt_next;  /* next pointer packet in pool */
};

/* Data Packet structure for command channel */
struct CCpacket {
	struct CCpacket *pk_next;/* next free packet in pool */
				/* only used by driver */
	int	pk_flag;	/* flags of this packet */
				/* only used by driver */
	uint	pk_fln;		/* forward link to another pointer */
				/* only used by firmware */
	unsigned char pk_cmd;	/* command */
	unsigned char pk_port;	/* serial port number */
	caddr_t	pk_sad;		/* start address */
	int	pk_len;		/* length of transfer */
	unsigned char pk_stat;	/* returned status */
	unsigned char pk_term;	/* terminating character */
	int	pk_rlen;	/* returned length */
};

/* codes for pk_flag */
#define PKBUSY	0x01		/* busy, not in free pool */
#define PKSTART 0x02		/* request started to ICC */
#define PKSLEEP	0x04		/* request sleeps and 
				   wait for completion */
#define PKWAKED	0x08		/* request waked up by interrupt */
#define PKCOMPL	0x10		/* request completed by interrupt */


/* codes for request mode */
#define MREAD	0x01	/* move the data from controller to driver */
#define MWRITE	0x02	/* move the data from driver to controller */
#define MSLEEP 0x04	/* sleep after starting request */

/* codes for the attention register */
#define ATTINTR	0x01	/* interrupt the controller */
#define ATTRESET 0x02	/* reset the controller */

/* SSM 07/22/86 - 2 */
/* new flag to remember wether readinterrupt occurred while line was blocked */
#define READBLOCKED 0200000
