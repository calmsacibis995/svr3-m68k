/*		Copyright (c) 1986 Motorola		*/
/*		All Rights Reserved				*/
/*
 *	mvme350.c: Motorola MVME350 VMEmodule Streamer Tape Controller driver
 *
 *	@(#)mvme350.c	6.4 modified
 *
 *	The MVME350 VMEmodule Streamer Tape Controller is an intelligent
 *	peripheral controller (IPC) VMEmodule designed to interface
 *	with the extended address line VMEmodules of the VMEbus family.
 *	The MVME350 supports only one streamer tape drive per controller
 *	using the QIC-02 Streaming Tape Interface.
 *
 *	This driver will support forward seeks on reads.
 */

/*	INCLUDE files 	*/

#include "sys/param.h"
#include "sys/fs/s5param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/buf.h"
#include "sys/signal.h"
#include "sys/fs/s5dir.h"
#include "sys/immu.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/file.h"
#include "sys/elog.h"
#include "sys/iobuf.h"
#include "sys/mvme350.h"
#include "sys/dbuf.h"
#include "sys/cmn_err.h"

#define	MULTITAPE	/* multiple tape support */
#define	STATCMD		/* do stat on open */
#define	BYTESWAP	/* include byteswap code */
#define	TAS		/* do tas instruction in getchan */
#undef	TAS

#define	DEBUG
#undef	DEBUG

#ifndef	M350
#define	M350	0
#endif

#ifdef	DEBUG		/* debugging code */

#define	STATIC
#define	DEBUGPR(x,d,s)	{ if( x & m350Debug ) m350print(d,s); }

int	m350Debug = 0;

#define	DBINIT		(1<<0)
#define	DBOPEN		(1<<1)
#define	DBBOPEN		(1<<2)
#define	DBCOPEN		(1<<3)
#define	DBCLOSE		(1<<4)
#define	DBBCLOSE	(1<<5)
#define	DBCCLOSE	(1<<6)
#define	DBSTRAT		(1<<7)
#define	DBSTART		(1<<8)
#define	DBINTR		(1<<9)
#define	DBREAD		(1<<10)
#define	DBWRITE		(1<<11)
#define	DBIOCTL		(1<<12)
#define	DBCLR		(1<<13)
#define	DBDUMP		(1<<14)
#define	DBDOIO		(1<<15)

#else			/* no debugging code */

#define	STATIC	static
#define	DEBUGPR(x,d,s)

#endif

#define SPLEV(X)  ( 0x2000 | ( X << 8 ))  /* convert interrupt level to splx()  
					     compatible arg */
#define	TOUT	2000			/* timeout */

extern int m350maxbsize;		/* maximum DMA buffer size */

/*
 *	MVME350 Driver Minor Number Assignments
 *
 *	/dev/mt and	Minor	Contr-		Rewind on	Append or
 *	/dev/rmt names	Number	oller 	Unit	close ?		Truncate ?
 *	--------------	------	------	----	---------	----------
 *	m350_0t		0	0	0	Yes		Truncate
 *	m350_0tn	1	0	0	No		Truncate
 *	m350_0a		2	0	0	Yes		Append
 *	m350_0an	3	0	0	No		Append
 *	m350_1t		4	1	0	Yes		Truncate
 *	m350_1tn	5	1	0	No		Truncate
 *	m350_1a		6	1	0	Yes		Append
 *	m350_1an	7	1	0	No		Append
 *
 *	m350_Xt		X*4	X	0	Yes		Truncate
 *	m350_Xtn	X*4+1	X	0	No		Truncate
 *	m350_Xa		X*4+2	X	0	Yes		Append
 *	m350_Xan	X*4+3	X	0	No		Append
 */

/*
 *	MVME350 Defines
 */

#define NORETENS(d)	(d & 0x80) 	/* dev -> no retension on open */
#define	CTL(d)		((d>>2)&0x1f)	/* dev -> controller */
#define	UNIT(d)		(0)		/* dev -> unit */
#define	POS(d)		((d>>2)&0x1f)	/* dev -> table position */
#define	RWD(d)		(!(d&1))	/* dev -> rewind flag */
#define	TDEV(d)		(!(d&2))	/* dev -> default to truncate */
#define	ADEV(d)		(d&2)		/* dev -> default to append */

#define	MAXUNIT	1

/*
 *	MVME350 CSR Space Structure
 *
 *	(m_fx bytes are for structure fill purposes)
 */

struct device {
	unsigned	m_addr;		/* IPC Address Register */
	unsigned char	m_am,	m_f0;	/* IPC Address Modifier Register */
	unsigned char	m_cr,	m_f1;	/* IPC Control Register */
	unsigned char	m_sr,	m_f2;	/* IPC Status Register */
	unsigned char	m_md,	m_f3;	/* IPC Model Data Register */
	unsigned char	m_av,	m_f4;	/* IPC Abort Vector Register */
	unsigned short	m_tas;		/* IPC TAS Register */
};

/*
 *	struct device defines
 *
 *	m_cr
 */

#define	ATTENTION	0x20	/* attention bit */
#define	SFINH		0x10	/* sysfail inhibit */

/*
 *	m_tas
 */

#define	TASBIT	0x8000		/* TAS reservation bit */
#define	VCMD	0x4000		/* valid command flag */
#define	VSTAT	0x2000		/* valid status flag */
#define	CCOMP	0x1000		/* command complete */
#define	TASCMD	0x0fff		/* TAS command mask */
#define	TASMASK	0xf000		/* TAS status mask */
#define	CREATE	0x0001		/* channel create command */
#define	DELETE	0x0002		/* channel delete command */

/*
 *	MVME350 Tape Composite Status Bit Assignments
 */

#define	S_FM		(1<<0)	/* Filemark detected. */
#define	S_BLOCK		(1<<1)	/* Block in error not located. */
#define	S_UNREC		(1<<2)	/* Unrecoverable data error. */
#define	S_EOM		(1<<3)	/* End of Media. */
#define	S_WP		(1<<4)	/* Write protected. */
#define	S_OFFLINE	(1<<5)	/* Drive not online. */
#define	S_NOCART	(1<<6)	/* Cartridge not in place. */
#define	S_BYTE0		(1<<7)	/* Exception occurred in byte 0. */
#define	S_RESET		(1<<8)	/* Reset/power up occurred. */
#define	S_BOM		(1<<11)	/* Beginning of Media. */
#define	S_RETRIES	(1<<12)	/* Eight or more retries. */
#define	S_NODATA	(1<<13)	/* No data detected. */
#define	S_ILLEGAL	(1<<14)	/* Illegal command. */
#define	S_BYTE1		(1<<15)	/* Exception occurred in byte 1. */
#define	S_NOFM		(1<<16)	/* No file mark encountered. */
#define	S_NOTBOT	(1<<17)	/* Not at beginning of tape. */
#define	S_TRESET	(1<<18)	/* Tape reset did not occur. */
#define	S_CTOD		(1<<20)	/* Controller-To-Device. */
#define	S_ITOC		(1<<21)	/* IPC-To-Controller. */
#define	S_STOI		(1<<22)	/* System-To-IPC. */
#define	S_TIMEOUT	(1<<23)	/* Timeout. */
#define	S_BADUNIT	(1<<24)	/* Bad Unit. */
#define	S_BADDEV	(1<<25)	/* Bad Device. */
#define	S_BADCMD	(1<<26)	/* Bad Command. */
#define	S_ABORT		(1<<27)	/* IPC Abort. */
#define	S_ETERM		(1<<28)	/* Early Termination. */
#define	S_CPUABORT	(1<<29)	/* CPU Abort. */
#define	S_FATAL		(1<<30)	/* Fatal Error. */
#define	S_ERROR		(1<<31)	/* Error. */

/*
 *	MVME350 Variables
 */

/*
 *	m350space.h
 */

extern	struct	iotime	m350stat[];
extern	struct	iobuf	m350tab[];

/*
 *	MVME350 iobuf usage
 *
 *	iobuf.h		mvme350.c	Usage
 *	-------		---------	-----
 *	b_flags		b_flags		Drive state
 *	b_forw
 *	b_back
 *	b_actf		b_actf		head (first) of I/O queue
 *	b_actl		b_actl		tail (last) of I/O queue
 *	b_dev		b_dev		major/minor device number
 *	b_active	b_active	busy flag
 *	b_errcnt	b_errcnt	error count
 *	io_erec		io_erec		error record
 *	io_nreg		io_nreg		number of registers to log upon error
 *	io_addr		io_addr		csr address
 *	io_mba		io_chan		channel structure address
 *	io_stp		io_stp		I/O statistics
 *	io_start	io_start	I/O statistics
 *	io_s1		io_bufp		buffer pointer for double buffering
 *	io_s2		io_bsize	buffer size
 *	io_s3		io_blkno	offset block number
 *	io_s4		io_nxrec	next record
 *	io_s5		io_pkt		I/O packet
 *	io_s6		io_fileno	file number on tape
 *	io_s7		io_ipc		ipc command buffer pointer
#ifdef	MULTITAPE
 *	io_s8(loword)	io_proc		process id
#else
 *	io_s8(loword)
#endif
*       io_s8(hiword)	io_moreflgs	more flags 
#ifdef	UNDERRUNS
*	io_s9		io_under	composite underrun count
#endif
 */

#define	io_chan		io_mba
#define	io_bufp		io_s1
#define	io_bsize	io_s2
#define	io_blkno	io_s3
#define	io_nxrec	io_s4
#define	io_pkt		io_s5
#define	io_fileno	io_s6
#define	io_ipc		io_s7
#define io_moreflgs	io_s8
#ifdef	MULTITAPE
#define	io_proc		io_s8
#endif
#ifdef	DEBUG
#define	UNDERRUNS
#define	io_under	io_s9
#endif

/* io_moreflgs definitions (upper word), loword is io_proc */
#define RDWROPEN 0x00010000		/* opened for read or write, not both*/ 
#define READONLY 0x00020000		/* reads only are permitted. */
#define WRITEONLY 0x00040000		/* writes only are permitted. */

#define	INF	0x7fffffff		/* a very large number */
#define	B_CMD	B_FORMAT
#define	B_INIT	B_FORMAT
#define	B_ERASE	B_DONE
#define	B_FM	B_AGE			/* filemark encountered */
#define	B_OFF	B_ASYNC			/* drive offline */
#define	B_READOP B_DELWRI		/* read operation done */
#ifdef	BYTESWAP
#define	B_BSWAP	B_STALE			/* byte swapping on */
#endif

#define	ENVPERCHAN	3		/* envelopes per channel */

extern	struct m350pkt	m350pkt[];	/* packets - one per controller */
extern	struct m350env	m350env[];	/* envelopes - three per controller */
extern	struct m350chan	m350chan[];	/* channels - one per controller */
extern	struct iostat	m350iostp[];	/* one per controller */

#define	INVALID	0
#define	VALID	1
#define	S32DATA	0x0d			/* address modifier - 32 bit */
#define	S24DATA	0x3d			/* address modifier - 24 bit */

/* MVME350 streamer tape commands */
#define	C_INIT	0x1001			/* init */
#define	C_READL	0x2001			/* read logical */
#define C_RIPC	0x2003			/* read ipc descriptor */
#define	C_STAT	0x2006			/* read status */
#define	C_REOF	0x2031			/* read to eof */
#define	C_WRTL	0x3001			/* write logical */
#define	C_WIPC	0x3003			/* write ipc descriptor */ 
#define	C_WEOF	0x3031			/* write eof */
#define	C_REW	0x4031			/* rewind */
#define	C_RET	0x4032			/* retension */
#define	C_ERASE	0x4033			/* erase */
#define	C_SEEK	0x4034			/* seek to eod */
#define	C_TREAD	0x4035			/* terminate read */

extern	int	m350packets;		/* number of packets */
extern	int	m350envelopes;		/* number of envelopes */


/*
 *	conf.h
 */

extern	struct	device	*m350_addr[];
extern	int		m350_ilev[];
extern	int		m350_ivec[];
extern	int		m350_cnt;
extern	int		m350_bmaj;
extern	int		m350_cmaj;

/*
 *	local variables
 */

/* ipc command buffer */
STATIC	struct m350ipc	m350ipc = { 0,0,0,0,0,0,0,0,0x200,0,0,0,0,0,0,0,0 };
static  initted = 0;

/*
 *	MVME350 Procedures
 *
 *	m350init: initialize the mvme350 and the driver
 */

m350init()
{	register struct iobuf *dp;
	register struct m350chan *cp;
	register c = m350_cnt/MAXUNIT;
	extern short cputype;		/* identifies processor */
	register am;
	register n;
	
	DEBUGPR(DBINIT,0,"init");

/*	am = (cputype == 682)? S32DATA : S24DATA;	*/
	am = S24DATA;

	for( n=0; n < c; n++ ) {
		if( bprobe( m350_addr[n], -1 ) )
			m350_addr[n] = NULL;
	}

	for( dp = &m350tab[0], n = 0; n < m350_cnt; n++, dp++ ) {
		if(( dp->io_addr = (paddr_t)m350_addr[n/MAXUNIT] ) == NULL )
			continue;
		dp->io_stp = &m350iostp[n];
		dp->io_nreg = sizeof(struct device);
		dp->io_chan = (physadr)&m350chan[n];
		dp->io_pkt = (int)&m350pkt[n];
		dp->io_ipc = (int)&m350ipc;
		dp->io_bsize = m350maxbsize;
		dp->io_fileno = 0;
		dp->b_flags = B_INIT;		/* init 350 first time */

		/*
		 *	initialize channel and envelopes
		 */

		cp = (struct m350chan *)dp->io_chan;
		cp->c_valid = INVALID;
		cp->c_level = m350_ilev[n];
		cp->c_vector = m350_ivec[n]/sizeof(long);
		cp->c_am = am;
		cp->c_prio = 0;

		{	register struct m350env *ep;

			ep = &m350env[n*ENVPERCHAN];
			ep->e_valid = INVALID;
			cp->c_cmd.p_head = ep;
			cp->c_cmd.p_tail = ep;
			ep->e_link = ep+1;
			(++ep)->e_valid = INVALID;
			(++ep)->e_valid = INVALID;
			cp->c_stat.p_head = ep;
			cp->c_stat.p_tail = ep;
		}

		if( m350getchan(dp) )
			dp->io_addr = NULL;

		{	register struct m350pkt *pp;

			pp = &m350pkt[n];

			pp->p_eye = 0x4d333530;		/* M350 eyecatcher */
			pp->p_cmdp = cp->c_chan;
			pp->p_statp = 0xff;
			pp->p_dtype = 2;	/* streamer tape */
			pp->p_am = cp->c_am;
/*			pp->p_busw = (cp->c_am == S24DATA)? 1 : 2;	*/
			pp->p_busw = (cputype == 682)? 2 : 1;
		}

	}
	initted = 1;
}

/*
 *	m350open: open an mvme350 device
 */

STATIC m350open(dev,flag,raw)
register dev;
register flag;
register raw;
{	register struct iobuf *dp = &m350tab[POS(dev)];
	register cstat;
	int	m350strategy();

	DEBUGPR(DBOPEN,dev,"open");

	/*
	 *	if the device is nonexistent, then error
	 */

	if((CTL(dev) > m350_cnt)||(dp->b_flags&B_OPEN)||(dp->io_addr == NULL)) {
		u.u_error = ENXIO;
		return;
	}

	/*
	 *	allow device to be opened for both read and write, but 
	 *	depending on first access, only allow read only or write only.
	 */

	if((flag&FREAD) && (flag&FWRITE)){
		dp->io_moreflgs &= ~( READONLY | WRITEONLY);
		dp->io_moreflgs |= RDWROPEN;
	}

	/*
	 *	set up the device table
	 */

	dp->b_flags &= ~B_OFF;
	dp->b_flags |= B_OPEN;
	dp->b_dev = makedev( m350_bmaj, dev) ;
	dp->io_blkno = 0;
	dp->io_nxrec = INF;

#ifdef BYTESWAP
	if (dp->b_flags & (B_INIT))  {		/* First open: 		*/
		dp->b_flags |= B_BSWAP;		/* set BYTEWSAPPING 	*/
		m350command(dp,C_WIPC,0);	/* wjc - 5/3/86 - MCS	*/
	}
#endif

#ifdef	STATCMD
	if(m350command(dp,C_STAT,0))
		u.u_error = 0;
	cstat = ((struct m350pkt *)dp->io_pkt)->p_cstat;
	/*
	 *	if the unit is offline and the "O_NDELAY" flag
	 *	isn't used, or if the unit is write protected and
	 *	the file is being opened for writing, then error.
	 */

	if(( ((flag&FNDELAY)==0) && ( cstat&(S_OFFLINE|S_NOCART) ))
	||(( (flag&FWRITE) && (cstat&S_WP) )) ) {
		dp->b_flags &= ~B_OPEN;
		u.u_error = ENXIO;
		return;
	}
#endif
	/* 	Retension tape for all write opens unless "NORETENS" bit 
	 *	is set in minor device number.
	 *
	 */
	if( (flag&FWRITE) && (! NORETENS(dev))) {
		if(m350command(dp, C_RET, 0)) {
			u.u_error = ENXIO;
			dp->b_flags &= ~B_OPEN;
			return;
		}
	}
	/*
	 *	if being opened for a write, check for truncate or append
	 */

	if( flag&FWRITE ) {
		
		/* set default action */
		if( (flag&(FTRUNC|FAPPEND)) == 0 )
			flag |= ADEV(dev)? FAPPEND : FTRUNC;

		if( flag&FTRUNC ) {	/* O_TRUNC == truncate ... */

			/*
			 *	force a rewind ...
			 *	can't overwrite on streamer tape
			 */

			if( m350command(dp,C_REW,0) ) {
				u.u_error = ENXIO;
				dp->b_flags &= ~B_OPEN;
				return;
			}

		/*
		 *	otherwise append ...
		 *	if the unit is being opened with the "O_APPEND" flag,
		 *	then position the tape at the end of data.
		 */

		} else if( m350command(dp,C_SEEK,-1) ) {
			u.u_error = ENXIO;
			dp->b_flags &= ~B_OPEN;
			return;
		}
	}

	/*
	 *	If the tape is opened as a character device (raw),
	 *	and if the O_NDELAY flag is not used, then allocate
	 *	a set of buffers to implement double buffering.
	 */

	if( raw && dp->io_bsize && ((flag&FNDELAY) == 0) ) {
#ifdef	MULTITAPE
		if( dp->io_bufp != NULL ) {
			if( u.u_procp->p_pid != loword(dp->io_proc) ) {
				dbufclose(dp->io_bufp);
				dp->io_bufp = NULL;
				printf("MVME350: DMA buffers discarded\n");
			} else {
				/* set up retry */
				dbufioctl( dp->io_bufp, DBUFRETRY, 0 );
			}
		} 

		if( dp->io_bufp == NULL ) {
#endif
			dp->io_bufp =
				dbufopen(dp->io_bsize,dp->b_dev,m350strategy);
			if( dp->io_bufp == NULL ) {
				dp->b_flags &= ~B_OPEN;
				return;
			}
#ifdef	MULTITAPE
		}
#endif
	}
#ifdef	MULTITAPE
	else if( dp->io_bufp ) {
		dbufclose(dp->io_bufp);
		dp->io_bufp = NULL;
		printf("MVME350: DMA buffers discarded\n");
	}

	loword(dp->io_proc) = u.u_procp->p_pid;
#endif


	DEBUGPR(DBOPEN,dev,"open successful");
}

/*
 *	m350bopen: open a block device
 */

m350bopen(dev,flag)
{
	DEBUGPR(DBBOPEN,dev,"bopen");
	m350open(dev,flag,0);
}

/*
 *	m350copen: open a character (raw) device
 */

m350copen(dev,flag)
{
	DEBUGPR(DBCOPEN,dev,"copen");
	m350open(dev,flag,1);
}

/*
 *	m350close: close an mvme350 device
 */

STATIC m350close(dev,flag,raw)
register dev;
{	register struct iobuf *dp = &m350tab[POS(dev)];

	DEBUGPR(DBCLOSE,dev,"close");

	if( (flag&FREAD) && (flag&FWRITE))
		dp->io_moreflgs &= ~(RDWROPEN | READONLY | WRITEONLY);

	/*
	 *	if the io_bufp is non-null, then flush and
	 *	delete the DMA buffers used for double buffering.
	 */

	if( dp->io_bufp ) {
#ifdef	DEBUG
		if( ! raw )
			m350print(dev,"illegal close");
#endif
#ifdef	MULTITAPE
		if( (flag&FREAD) || ((dp->b_flags&B_ERROR) == 0) ) {
#endif
			dbufclose(dp->io_bufp);
			dp->io_bufp = NULL;
#ifdef	MULTITAPE
		} else {
			dp->b_flags &= ~B_ERROR;
			printf("MVME350: DMA buffers still active.\n");
		}
#endif
	}

	/*
	 *	if the unit was opened for writing, then write
	 *	a filemark to indicate end of file
	 */

	if( dp->b_flags&B_ERASE )
		dp->b_flags &= ~B_ERASE;

	else if( flag&FWRITE )
		m350command(dp,C_WEOF,0);

	/*
	 *	if the unit is a "rewind-on-close", then rewind the device
	 */

	/* if read and not at filemark terminate read */
	if((flag&FREAD) && ((dp->b_flags&B_FM) == 0)) {
		if( RWD(dev) )
			m350command(dp,C_INIT,0);	/* do init for rewind */
		else if (dp->b_flags&B_READOP) {	/* if last op a read  */
			m350command(dp,C_TREAD,0);
			dp->b_flags &= ~B_READOP;
			}
	}

	if( RWD(dev) )
		m350command(dp,C_REW,0);

	dp->b_flags &= ~B_OPEN;

	DEBUGPR(DBCLOSE,dev,"close successful");
}

/*
 *	m350bclose: close a block device
 */

m350bclose(dev,flag)
{
	DEBUGPR(DBBCLOSE,dev,"bclose");
	m350close(dev,flag,0);
}

/*
 *	m350cclose: close a character device
 */

m350cclose(dev,flag)
{
	DEBUGPR(DBCCLOSE,dev,"cclose");
	m350close(dev,flag,1);
}

/*
 *	m350command: issue a command and call strategy
 */

STATIC m350command(dp,cmd,arg)
register struct iobuf *dp;
register cmd;
register arg;
{	register struct buf *bp;

	bp = geteblk();			/* get empty buffer */
	bp->b_proc = u.u_procp;
	bp->b_dev = dp->b_dev;
	bp->b_bcount = cmd;
	bp->b_blkno = arg;
	bp->b_error = 0;
	bp->b_flags |= B_CMD|B_PHYS;
	m350strategy(bp);
	iowait(bp);			/* wait until done */
	bp->b_flags &= ~(B_CMD|B_PHYS);
	brelse(bp);
	return(u.u_error);
}

/*
 *	m350strategy: perform an I/O
 */

m350strategy(bp)
register struct buf *bp;
{	register struct iobuf *dp = &m350tab[POS(bp->b_dev)];

	DEBUGPR(DBSTRAT,bp->b_dev,"strategy");

	if(((bp->b_flags&B_CMD ) == 0) && (bp->b_bcount&(BSIZE-1))) {
		/* no partial block I/O */
		bp->b_flags |= B_ERROR;
		bp->b_error = ENXIO;
		iodone(bp);
		return;
	}

	 /* if device was opened for read/write then depending on first access,
	    permit reads only or writes only. 
	 */
	if(((bp->b_flags&B_CMD) == 0) && (dp->io_moreflgs & RDWROPEN)){
		if(!(dp->io_moreflgs & (READONLY | WRITEONLY))){
			if(bp->b_flags & B_READ)
				dp->io_moreflgs |= READONLY;
			else
				dp->io_moreflgs |= WRITEONLY;
		}
		else{
	     	      if(!(((bp->b_flags&B_READ) && (dp->io_moreflgs&READONLY))
		    || (((bp->b_flags&B_READ) == 0) && (dp->io_moreflgs&WRITEONLY))))
		        {	 /* cant mix reads and writes */
				bp->b_flags |= B_ERROR;
				bp->b_error = ENXIO;
				iodone(bp);
				return;
			}
		}
	}
	bp->b_start = lbolt;

	if(( bp->b_flags&B_CMD ) == 0 )
		m350stat[POS(bp->b_dev)].io_bcnt += btoc(bp->b_bcount);
	bp->av_forw = NULL;

	splx(SPLEV(m350_ilev[POS(bp->b_dev)]));

	if( dp->b_actf == NULL )
		dp->b_actf = bp;
	else
		dp->b_actl->av_forw = bp;
	dp->b_actl = bp;
	if( dp->b_active == 0 )
		m350start(dp);

	spl0();
}

/*
 *	m350start: start an I/O
 */

STATIC m350start(dp)
register struct iobuf *dp;
{	register struct buf *bp;
	register struct m350env *ep;
	struct m350chan *cp = (struct m350chan *)dp->io_chan;
	register cmd;
	register bpblkno;

	DEBUGPR(DBSTART,dp->b_dev,"start");

	/*
	 *	get next buffer ... if null return
	 */

	while(( bp = dp->b_actf ) != NULL ) {

		dp->b_active++;

		if(( bp->b_flags&B_CMD ) == 0 ) {

			/*
			 *	if the current block number is greater
			 *	than the next record number on the tape,
			 *	then it's an error (lseek(2) too far).
			 */

			if( (bpblkno = bp->b_blkno) > dp->io_nxrec ) {
				bp->b_error = EIO;
				goto abort;
			}

			/*
			 *	if the current block is equal to the
			 *	next record number, then we're at the
			 *	end of the tape file (on a filemark).
			 */

			if((bpblkno==dp->io_nxrec)&&(bp->b_flags&B_READ)) {
				clrbuf(bp);
				bp->b_resid = bp->b_bcount;
				goto next;
			}

			/*
			 *	set the next record number to one past the
			 *	buffer block number (on writes).
			 */

			if(( bp->b_flags&B_READ ) == 0 ) {
				dp->io_nxrec = bpblkno + bp->b_bcount/BSIZE;

				/*
				 *	no seeks on write
				 */

				if( dp->io_blkno != bp->b_blkno ) {
#ifdef	DEBUG
					printf("m350start: unexpected write\n");
#endif
					bp->b_error = EIO;
					goto abort;
				}
			} else if( dp->io_blkno > bpblkno ) {
				bp->b_error = ENXIO;
				goto abort;
			}

		}

		if( dp->b_flags&B_OFF )
			goto abort;

		/*
		 *	if command, then set command
		 */

		if( dp->b_flags&B_INIT )
			cmd = C_INIT;
		else if( (dp->b_flags&B_ONCE) == 0 )
			cmd = C_WIPC;
		else if( bp->b_flags&B_CMD )
			cmd = bp->b_bcount;	/* see m350command */
		else
			cmd = (bp->b_flags&B_READ)? C_REOF : C_WRTL;

		/*
		 *	set up packet
		 */

	{	register struct m350pkt *pp = (struct m350pkt *)dp->io_pkt;

		pp->p_cmd = cmd;

		switch(cmd) {

		case C_REOF:	/* read to eof */
			pp->p_praddr = dp->io_fileno;
			dp->b_flags |= B_READOP;	/* read done */

		case C_WRTL:	/* write logical */

			if( bp->b_flags&B_PHYS ) {
				dp->io_stp->io_ops++;

			} else if( dp->io_blkno == bpblkno ) {
				dp->io_stp->io_ops++;

			} else	/* do block read and throw away later */
				dp->io_stp->io_misc++;

			pp->p_scaddr = (unsigned)paddr(bp);
			pp->p_count = bp->b_bcount/BSIZE;
			if( dp->io_blkno < bp->b_blkno ) {
				/* seek forward ... but not too far */
				if(pp->p_count > (bp->b_blkno-dp->io_blkno))
					pp->p_count = bp->b_blkno-dp->io_blkno;
			}
			break;

		case C_WIPC:
#ifdef	BYTESWAP
			if( dp->b_flags&B_ONCE ) 
				pp->p_dtype = 0x2;	/* tape device type */
			else
#endif
				pp->p_dtype = 0xf;	/* IPC device type */
			pp->p_scaddr = dp->io_ipc;
#ifdef	BYTESWAP
			pp->p_res2 = ((dp->b_flags&B_BSWAP) == B_BSWAP);
#endif
			break;

		case C_WEOF:	/* write eof */
		case C_TREAD:	/* terminate read */
			pp->p_count = 0;
			dp->io_stp->io_misc++;
			break;

#ifdef	STATCMD
		case C_STAT:	/* get status */
#endif
		case C_INIT:	/* init */
		case C_REW:	/* rewind */
		case C_RET:	/* retension */
		case C_ERASE:	/* erase */
			dp->io_fileno = 0;
			dp->io_stp->io_misc++;
			break;

		case C_SEEK:	/* seek to eod */
			dp->io_stp->io_misc++;
			pp->p_praddr = bp->b_blkno;
			break;
#ifdef	DEBUG
		default:
			printf("MVME350 bad command: %x\n", cmd );
			break;
#endif
		}

		blkacty |= (1<<M350);

		/* set up I/O start time */
		if( dp->io_start == 0 )
			dp->io_start = lbolt;
		 
		/*
		 *	set up channel
		 */



		ep = cp->c_cmd.p_tail;
		cp->c_cmd.p_tail = ep->e_link;
		ep->e_pkt = pp;
	}

		/*
		 *	everything ok ... set attention
		 */

	{	register struct device *rp = (struct device *)dp->io_addr;

		ep->e_valid = VALID;
		rp->m_cr |= ATTENTION;
		return;
	}

		/*
		 *	come here on error
		 */

	abort:
		bp->b_flags |= B_ERROR;
	next:
		dp->b_actf = bp->av_forw;
		iodone(bp);
	}

	dp->b_active = 0;
}

/*
 *	m350intr: interrupt handler
 */

m350intr(ctl)
{	register struct iobuf *dp = &m350tab[ctl];
	register struct buf *bp = dp->b_actf;
	register struct m350pkt *pp;
	register dev = bp->b_dev;
	register resid;
	register stat;
	register cmd;

	DEBUGPR(DBINTR,dev,"intr");

	{	
		struct m350chan *cp = (struct m350chan *)dp->io_chan;
		register struct m350env *ep;
		struct m350env *sep;

		/*
		 *	get status envelope
		 */

#ifdef	DEBUG
		if(m350_addr[ctl] == NULL)	
			return;
#endif

		ep = cp->c_stat.p_head;

		if( ep->e_valid == INVALID ) {
#ifdef	DEBUG
			m350print(dev,"spurious interrupt");
#endif
			return;
		}

		ep->e_valid = INVALID;

#ifdef	DEBUG
		if( ep->e_pkt == NULL )
			cmn_err(CE_PANIC, "MVME350: null status");
#endif
		/*
		 *	get status
		 */

		pp = ep->e_pkt;

		ep->e_pkt = NULL;

		/*
		 *	clean up envelope queues
		 */

		sep = cp->c_cmd.p_tail;

#ifdef	DEBUG
		if( sep == NULL )
			cmn_err(CE_PANIC, "MVME350: Corrupted envelopes");
#endif
		sep->e_link = ep;
		cp->c_cmd.p_head = sep;

		sep = ep->e_link;
		cp->c_stat.p_head = sep;
		cp->c_stat.p_tail = sep;

		ep->e_link = NULL;
	}

	if( bp == NULL ) {
		logstray(dp->io_addr);
		return;
	}

	blkacty &= ~(1<<M350);
	dp->b_active = 0;

	dp->io_fileno = pp->p_fileno;
	cmd = pp->p_cmd;

	if((stat = pp->p_cstat)&S_ERROR) {

		if(( stat&S_EOM ) && (( cmd == C_WRTL )) || ( cmd == C_WEOF ))
			stat &= ~S_FATAL;

		else if((stat&S_FM) == 0) {
			/* print error message(s) */
			m350prerr(dev,cmd,stat);
			dp->b_dev = makedev(M350, minor(bp->b_dev));
			fmtberr(dp,stat);
			if( stat&S_OFFLINE )
				dp->b_flags |= B_OFF;
		}
	}

	/*
	 *	if fatal error
	 */

	if( stat&S_FATAL ) {
		dp->b_flags |= B_INIT;
		pp->p_termcnt = 0;
		bp->b_flags |= B_ERROR;

	/*
	 *	if command is a write IPC
	 */

	} else if( cmd == C_WIPC ) {
		pp->p_dtype = 0x2;
		if(( dp->b_flags&B_ONCE) == 0) {	/* initial WIPC */
			dp->b_flags |= B_ONCE;		/* changing blocksize */
			m350start(dp);
			return;
		}
	
	
	/*
	 *	if an init command has been issued
	 */

	} else if( dp->b_flags&(B_INIT) ) {
		dp->b_flags &= ~(B_INIT);
		m350start(dp);		/* do command */
		return;

	/*
	 *	if the command is a read or write
	 */

	} else if((cmd == C_REOF ) || (cmd == C_WRTL )) {
		resid = pp->p_termcnt/BSIZE;
#ifdef	UNDERRUNS
		dp->io_under += pp->p_under;
#endif
		dp->io_blkno += resid;
		if( dp->io_blkno <= bp->b_blkno ) {
			m350start(dp);	/* ignore I/O ... continue */
			return;
		}
		if( stat&S_FM ) {
			dp->io_blkno = 0;
			dp->io_nxrec = bp->b_blkno + resid;
		}
	}

	/*
	 *	I/O is complete ... continue
	 */

	logberr(dp,stat);
	dp->b_actf = bp->av_forw;

	if(((bp->b_flags&B_CMD ) == 0) && 
	(!(stat&S_FATAL) && ((cmd == C_WRTL ) || (cmd == C_REOF)))) {
		resid = bp->b_bcount-pp->p_termcnt;
		bp->b_resid = (resid <= 0)? 0 : resid;
		if( stat & S_FM )
			dp->b_flags |= B_FM;
		else
			dp->b_flags &= ~B_FM;

	} else if((bp->b_flags&B_CMD ) == 0) 
		bp->b_resid = bp->b_bcount;


	{	register struct iotime *ip = &m350stat[ctl];

		ip->io_resp += lbolt - bp->b_start;
		ip->io_act += lbolt - dp->io_start;
		dp->io_start = 0;
	}

	iodone(bp);		/* complete the I/O */
#ifndef	STATCMD
	pp->p_cstat = 0;
#endif
	m350start(dp);		/* start next I/O */
}

m350breakup(bp)
{
	dma_breakup(m350strategy,bp,BSIZE);
}

/*
 *	m350read: character device read
 */

m350read(dev)
register dev;
{	register struct iobuf *dp = &m350tab[POS(dev)];

	DEBUGPR(DBREAD,dev,"read");

	if(dp->io_moreflgs&RDWROPEN){
		if(dp->io_moreflgs&WRITEONLY){
			u.u_error = ENXIO;
			return;
		}
		else
			dp->io_moreflgs |= READONLY;
	}
	
	if( dp->io_bufp )
		dbufread( dp->io_bufp );
	else
		physio( m350breakup, 0, dev, B_READ );
}

/*
 *	m350write: character device write
 */

m350write(dev)
{	register struct iobuf *dp = &m350tab[POS(dev)];

	DEBUGPR(DBWRITE,dev,"write");

	if(dp->io_moreflgs&RDWROPEN){
		if(dp->io_moreflgs&READONLY){
			u.u_error = ENXIO;
			return;
		}
		else
			dp->io_moreflgs |= WRITEONLY;
	}
	
	if( dp->io_bufp ) {
		dbufwrite( dp->io_bufp );
#ifdef	MULTITAPE
		if( u.u_error || u.u_count != 0 )
			dp->b_flags |= B_ERROR;	/* remember error */
#endif
	} else
		physio( m350breakup, 0, dev, B_WRITE );
}

/*
 *	m350ioctl: ioctl support for the MVME350
 */

m350ioctl( dev, cmd, arg, mode )
register dev;
{	register struct iobuf *dp = &m350tab[POS(dev)];

	DEBUGPR(DBIOCTL,dev,"ioctl");

	switch(cmd) {
	case M350REWIND:	/* rewind tape */
		m350command(dp,C_REW,0);
		break;
	case M350ERASE:		/* erase tape */
		if( m350command(dp,C_ERASE,0) == 0 )
			dp->b_flags |= B_ERASE;
		break;
	case M350RETENSION:	/* retension tape */
		m350command(dp,C_RET,0);
		break;
	case M350WRTFM:		/* write filemark */
		m350command(dp,C_WEOF,0);
		break;
	case M350RDFM:		/* read filemark */
		m350command(dp,C_SEEK,dp->io_fileno);
		break;
#ifdef	BYTESWAP
	case	M350BYTESWAP:	/* set/reset byte swapping */
		if( arg != 0 )
			dp->b_flags |= B_BSWAP;
		else
			dp->b_flags &= ~B_BSWAP;
		m350command(dp,C_WIPC,0);
		break;
#endif
	case M350SETDMA:	/* set DMA buffer size */
		if( ! suser() ) {
			u.u_error = EPERM;
			return;
		}

		if( dp->io_bufp == NULL )
			dp->io_bsize = arg;

		else {
			dbufioctl(dp->io_bufp,DBUFSET,arg);
			if( u.u_error )
				dp->io_bsize = 0;
			else
				dp->io_bsize = arg;
		}
		break;
	case M350GETDMA:	/* get DMA buffer size */
		if(( dp->io_bufp == NULL ) || ( dp->io_bsize == 0 )) {
			copyout( &dp->io_bsize, arg, sizeof(dp->io_bsize) );
		} else
			dbufioctl(dp->io_bufp,DBUFGET,arg);
		break;

#ifdef	UNDERRUNS
	case (M350IOCTL|'U'):	/* get underrun count */
		copyout( &dp->io_under, arg, sizeof( dp->io_under ) );
		break;
#endif

	default:
		u.u_error = EINVAL;
		break;
	}
}

/*
 *	m350clr: clear the mvme350 after a power fail
 */

m350clr()
{
	DEBUGPR(DBCLR,0,"clr");
}

/*
 *	m350print: driver print procedure
 */

m350print(d,s)
{	printf("MVME350: %s on controller %d, drive %d\n", s, CTL(d), UNIT(d) );
}

/*
 *	m350prerr: print an MVME350 error message
 */

STATIC m350prerr(dev,cmd,stat)
{
	if( (stat&S_ERROR) == 0 )
		return;
	m350print(dev,"Error");
	m350prcmd(cmd);
	m350prstat(stat);
}

/*
 *	m350prcmd: print MVME350 command
 */

m350prcmd(cmd)
register unsigned short cmd;

{	switch(cmd) {
	case C_INIT:	m350prm("Initialize device");	break;
	case C_READL:	m350prm("Read logical");		break;
	case C_STAT:	m350prm("Read status");		break;
	case C_REOF:	m350prm("Read blocks");		break;
	case C_WRTL:	m350prm("Write blocks");		break;
	case C_WIPC:	m350prm("Write ipc descriptor");	break;
	case C_WEOF:	m350prm("Write eof");		break;
	case C_REW:	m350prm("Rewind tape");		break;
	case C_RET:	m350prm("Retension tape");		break;
	case C_ERASE:	m350prm("Erase tape");		break;
	case C_SEEK:	m350prm("Seek to end-of-data");	break;
	case C_TREAD:	m350prm("Terminate read");		break;
	default:
		printf("MVME350: Unknown command %x.\n", cmd );
		break;
	}
}

/*
 *	m350prstat: print MVME350 composite status
 */

STATIC m350prstat(stat)
register unsigned stat;

{
	if( stat == 0 ) {
		m350prm("No error");
		return;
	}

	if (stat&S_FM)		m350prm("Filemark detected");
#ifdef	DEBUG
	if (stat&S_BLOCK)	m350prm("Block in error not located");
#endif
	if (stat&S_UNREC)	m350prm("Unrecoverable data error");
	if (stat&S_EOM)		m350prm("End of Media");
	if (stat&S_WP)		m350prm("Write protected");
	if (stat&S_OFFLINE)	m350prm("Drive not online");
	if (stat&S_NOCART)	m350prm("Cartridge not in place");
#ifdef	DEBUG
	if (stat&S_BYTE0)	m350prm("Exception occurred in byte 0");
	if (stat&S_RESET)	m350prm("Reset/power up occurred");
#endif
	if (stat&S_BOM)		m350prm("Beginning of Media");
#ifdef	DEBUG
	if (stat&S_RETRIES)	m350prm("Eight or more retries");
#endif
	if (stat&S_NODATA)	m350prm("No data detected");
#ifdef	DEBUG
	if (stat&S_ILLEGAL)	m350prm("Illegal command");
	if (stat&S_BYTE1)	m350prm("Exception occurred in byte 1");
#endif
	if (stat&S_NOFM)	m350prm("No file mark encountered");
	if (stat&S_NOTBOT)	m350prm("Not at beginning of tape");
	if (stat&S_TRESET)	m350prm("Tape reset did not occur");
#ifdef	DEBUG
	if (stat&S_CTOD)	m350prm("Controller-To-Device");
	if (stat&S_ITOC)	m350prm("IPC-To-Controller");
	if (stat&S_STOI)	m350prm("System-To-IPC");
#endif
	if (stat&S_TIMEOUT)	m350prm("Timeout");
	if (stat&S_BADUNIT)	m350prm("Bad Unit");
	if (stat&S_BADDEV)	m350prm("Bad Device");
#ifdef	DEBUG
	if (stat&S_BADCMD)	m350prm("Bad Command");
	if (stat&S_ABORT)	m350prm("IPC Abort");
	if (stat&S_ETERM)	m350prm("Early Termination");
	if (stat&S_CPUABORT)	m350prm("CPU Abort");
	if (stat&S_FATAL)	m350prm("Fatal Error");
	if (stat&S_ERROR)	m350prm("Error");
#endif
}

/*
 *	m350prm: print message with MVME: at front
 */

STATIC m350prm(s)
{	printf("MVME350: %s.\n", s );
}

/*
 *	m350dump: dump memory to tape
 */

m350dump()
{	
	int m350doio();
	extern	dev_t dumpdev;
	register dev = dumpdev;
	register struct iobuf *dp = &m350tab[POS(dev)];
	register struct m350pkt *pp = (struct m350pkt *)dp->io_pkt;

	DEBUGPR(DBDUMP,dev,"dump");

	if((CTL(dev) > m350_cnt)) {
		printf("Dump device controller not present\n");
		return;
	}
	spl7();
	if(initted == 0)
		m350init();
	/* 
	 *	init vme350
	 */
	
	pp->p_cmd = C_INIT;
	m350sendpkt(pp,dev);
	dp->io_blkno = INF - 1;
/*	memdump(m350doio);	*/
	pp->p_cmd = C_WEOF;
	m350sendpkt(pp,dev);
	pp->p_cmd = C_REW;
	m350sendpkt(pp,dev);
}

/*
 *	m350doio: write one block of memory
 */

STATIC m350doio(dev,blkno,maddr,rdwr)
register dev;		/* device # */
register blkno;		/* block number on disk (512-byte) */
register char *maddr;	/* memory address */
register rdwr;		/* B_READ or B_WRITE */
{	register struct iobuf *dp = &m350tab[POS(dev)];
	register struct m350pkt *pp = (struct m350pkt *)dp->io_pkt;
	register i;

	DEBUGPR(DBDOIO,dev,"doio");

	/* set up packet */
	pp->p_cmd = (rdwr == B_READ) ? C_REOF : C_WRTL;
	pp->p_praddr = 0;
	pp->p_scaddr = (unsigned) maddr;
	pp->p_count = 1;		/* one block */
	m350sendpkt(pp,dev);
	if(dp->io_blkno + 1 < blkno)	/* skip hole */
		for(i=dp->io_blkno+1;i<blkno;i++)
			m350sendpkt(pp,dev);
	dp->io_blkno = blkno;
}


/****************************************************************
 *								*
 *	MVME350 Control Procedures				*
 *								*
 ****************************************************************/

/*
 *	m350sendpkt - send packet to vme350
 */

STATIC m350sendpkt(pp,dev)
register struct m350pkt *pp;
register dev;
{	register struct iobuf *dp = &m350tab[POS(dev)];
	register struct m350chan *cp = (struct m350chan *)dp->io_chan;
	register struct m350env *ep;
	struct m350env *sep;
	struct device *rp = (struct device *)dp->io_addr;
	register tout = INF;

	/* set up envelope */
	ep = cp->c_cmd.p_tail;
	cp->c_cmd.p_tail = ep->e_link;
	ep->e_pkt = pp;
	ep->e_valid = VALID;
	ep = cp->c_stat.p_tail;
	rp->m_cr |= ATTENTION;
	
	/*
	 * wait till packet put on status tail 
	 */

	while (ep->e_link == NULL && --tout > 0)
		;

	if(tout <= 0) {
#ifdef	DEBUG
		m350print(dev,"timeout while waiting for completion");
#endif
		return;
	}
	/*
	 *	get status envelope
	 */
	ep = cp->c_stat.p_head;
	while( ep->e_valid == INVALID ) 
		;

	ep->e_valid = INVALID;

#ifdef	DEBUG
	if( ep->e_pkt == NULL )
		cmn_err(CE_PANIC, "MVME350: null status");
#endif
	/*
	 *	get status
	 */

	pp = ep->e_pkt;

	ep->e_pkt = NULL;

	/*
	 *	clean up envelope queues
	 */

	sep = cp->c_cmd.p_tail;

#ifdef	DEBUG
	if( sep == NULL )
		cmn_err(CE_PANIC, "MVME350: Corrupted envelopes");
#endif
	sep->e_link = ep;
	cp->c_cmd.p_head = sep;

	sep = ep->e_link;
	cp->c_stat.p_head = sep;
	cp->c_stat.p_tail = sep;

	ep->e_link = NULL;

	if( pp->p_cstat&S_ERROR ) 		/* an error occurred */
		/* print error message(s) */
		m350prerr(dev,pp->p_cmd,pp->p_cstat);

}

/*
 *	m350getchan: set up channel
 */

STATIC m350getchan(dp)
register struct iobuf *dp;
{	register struct device *rp = (struct device *)dp->io_addr;
	register struct m350chan *cp = (struct m350chan *)dp->io_chan;
	register tout;		/* timeout */

#ifdef	TAS
	for( tout = TOUT; tout &&  ! m350tas(&rp->m_tas); tout-- );

	if( tout <= 0 ) {
		printf("MVME350: initialization error\n" );
		return(1);	/* error */
	}
#else
	/*
	 *	try to protect ourselves as much as possible
	 */

	for( tout = TOUT; tout && (rp->m_tas&TASBIT); tout-- );

	if( tout <= 0 ) {
		printf("MVME350: initialization error\n" );
		return(1);	/* error */
	}

	rp->m_tas |= TASBIT;	/* not safe like a real TAS */
#endif

	/*
	 *	set up register file
	 */

	rp->m_am = cp->c_am;
	rp->m_addr = (unsigned)cp;
	rp->m_tas = (rp->m_tas&TASMASK) | VCMD | CREATE;

	/*
	 *	tell MVME350 that command is ready
	 */

	rp->m_cr |= ATTENTION;

	/*
	 *	wait for VSTAT bit
	 */

	for( tout=TOUT; tout && ((rp->m_tas&VSTAT) == 0); tout-- );

	if( tout <= 0 ) {
		printf("MVME350: initialization error\n" );
		return(1);	/* error */
	}

	if( rp->m_sr ) {
		printf("MVME350: initialization error %x\n", rp->m_sr );
		return(1);	/* error */
	}

	/*
	 *	indicate command complete
	 */

	rp->m_tas |= CCOMP;

	/*
	 *	now tell MVME350
	 */

	rp->m_cr |= ATTENTION;

	return(0);		/* no error */
}

#ifdef	TAS

/*
 *	m350tas - test and set ipc tas register
 *
 *	WARNING: sometimes the optimizer causes problems
 *			with the following asm()'s.
 */

STATIC m350tas(tp)
short *tp;
{
	asm(" mov.l	8(%fp),%a0	");
	asm(" clr.l	%d0		");
	asm(" tas.b	(%a0)		");
	asm(" bmi.b	tasdone		");
	asm(" mov.l	&1,%d0		");
	asm(" tasdone:			");
}

#endif
