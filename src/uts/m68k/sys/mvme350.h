/*
 *	mvme350.h: include file for mvme350
 *
 *	%W%
 */

/*
 *	MVME350 packet structure
 */

struct m350pkt {
	unsigned	p_eye;		/* eyecatcher */
	unsigned char	p_cmdp;		/* command pipe number */
	unsigned char	p_statp;	/* status pipe number */
	unsigned	p_num;		/* packet number */
	unsigned short	p_cmd;		/* command */
	unsigned char	p_dtype;	/* device type */
	unsigned char	p_res1;		/* reserved */
	unsigned char	p_am;		/* address modifier */
	unsigned char	p_busw;		/* bus width */
	unsigned char	p_punit;	/* primary unit */
	unsigned char	p_sunit;	/* secondary unit */
	unsigned	p_praddr;	/* prim address */
	unsigned	p_scaddr;	/* secondary address */
	unsigned	p_count;	/* transfer count */
	unsigned short	p_res2;		/* reserved */
	unsigned short	p_res3;		/* reserved */
	unsigned short	p_res4;		/* reserved */
	unsigned	p_cstat;	/* composite status */
	unsigned short	p_res5;		/* reserved */
	unsigned char	p_sberror;	/* system bus error status */
	unsigned char	p_lberror;	/* local bus error status */
	unsigned short	p_retry;	/* retry count */
	unsigned	p_pstat;	/* primary status address */
	unsigned	p_sstat;	/* secondary status address */
	unsigned	p_termcnt;	/* termination transfer count */
	unsigned short	p_under;	/* underrun count */
	unsigned short	p_curop;	/* current operation */
	unsigned short	p_fileno;	/* file number (tape position) */
};

/*
 *	MVME350 Envelope
 */

struct m350env {
	struct	m350env	*e_link;	/* next envelope */
	struct	m350pkt	*e_pkt;		/* packet pointer */
	unsigned char	e_valid;	/* valid flag */
	unsigned char	e_res0;		/* reserved */
	unsigned short	e_res;		/* reserved */
};

/*
 *	MVME350 pipe
 */

struct m350pipe {
	struct	m350env	*p_head;	/* pipe head pointer */
	struct	m350env	*p_tail;	/* pipe tail pointer */
};

/*
 *	MVME350 channel structure
 */

struct m350chan {
	struct m350pipe	c_cmd;		/* command pipe */
	struct m350pipe	c_stat;		/* status pipe */
	unsigned char	c_level;	/* interrupt level */
	unsigned char	c_vector;	/* interrupt vector */
	unsigned char	c_prio;		/* channel priority */
	unsigned char	c_am;		/* address mask */
	unsigned char	c_chan;		/* channel number */
	unsigned char	c_valid;	/* valid channel flag */
};

/*
 *	MVME350 IPC Descriptor
 */

struct m350ipc {
	unsigned i_model1;		/* model number */
	unsigned i_model2;		/* model number */
	unsigned i_date1;		/* date of release */
	unsigned i_date2;		/* date of release */
	unsigned short i_rel;		/* release number */
	unsigned short i_level;		/* release level */
	unsigned short i_aen;		/* age enable */
	unsigned short i_vblk;		/* VMEbus DMA block size */
	unsigned short i_lblk;		/* logical block size */
	unsigned short i_max_buf;	/* not used by mvme350 firmware */
	unsigned short i_halffull;	/* high water mark for queues */
	unsigned short i_fastslice;	/* timer ticks */
	unsigned short i_slowslice;	/* timer ticks */
	unsigned short i_timlimt;	/* for aging ready queues */
	unsigned short i_heartbeat;	/* length of timer tick */
	unsigned short i_res;		/* reserved */
	unsigned short i_packets;	/* packet count */
};

/*
 *	MVME350 IOCTL support
 */

#define	M350IOCTL	('M'<<8)
#define	M350REWIND	(M350IOCTL|1)	/* rewind tape */
#define	M350ERASE	(M350IOCTL|2)	/* erase tape */
#define	M350RETENSION	(M350IOCTL|3)	/* retension tape */
#define	M350WRTFM	(M350IOCTL|4)	/* write filemark */
#define	M350RDFM	(M350IOCTL|5)	/* read filemark */
#define	M350SETDMA	(M350IOCTL|6)	/* set DMA buffer size */
#define	M350GETDMA	(M350IOCTL|7)	/* get DMA buffer size */
#define	M350BYTESWAP	(M350IOCTL|8)	/* set/reset byteswapping */
