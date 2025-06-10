/*		Copyright (c) 1985 AT&T		*/
/*		All Rights Reserved		*/
/* @(#)mvme320.c	8.4  */

/********************************************************************
*
*  A UNIX driver for the mvme320 disk controller. Prepared IAW
*  the revised and corrected controller	manual dated 2 Oct 84.
*
*  Features of this driver include:
*
*    - Dynamic disk configuration (actual heads	and cylinders)
*      based on	a VERSAdos (-like) volume label.
*
*    - Bad track detection and alternate assignment using the
*      software	method developed by Microsystems.
*
********************************************************************/

#define	DEBUG
#undef	DEBUG 
#define DEBUGERR
#undef  DEBUGERR
#define DEBUGQ
#undef  DEBUGQ


#ifdef DEBUG
int M320Debug = 0x0;

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

#endif

#define	 LOCAL static	/* make	null for debug loadmap */

/********************************************************************
  UNIX system includes
*/

#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/fs/s5dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/buf.h"
#include "sys/elog.h" 
#include "sys/iobuf.h" 
#include "sys/systm.h"
#include "sys/dk.h" 
#include "sys/open.h"

/********************************************************************
  device-specific includes
*/

#include "sys/mvme320.h" 

/********************************************************************
  miscellaneous	stuff which ought to be	in includes but	isn't
*/

#define SPLEV(level)  ( 0x2000 | ( level << 8 ))  /* convert int level to  
					     splx() compatible argument */

struct size {
	daddr_t	nblocks;	/* size of slice in blocks */
	daddr_t	blkoff;		/* first physical block of slice */
};

/********************************************************************
  UNIX iobuf and buf overlays, locally-defined buf flags
*/

/* controller iobuf */

#define	Busy	b_active	/* controller busy flag/timeout counter */
#define	Ioaddr	io_s3		/* controller csr base addr */
#define	Request	io_s4		/* ptr to active buf */
#define	Sector	io_s5		/* read/write, current sector */
#define	Track	io_s5		/* format, current track */
#define	Buffer	io_s6		/* working buffer address */
#define	Count	io_s7		/* working byte/track count */
#define	Rcount	io_s8		/* byte count of last request */
#define Kluge   io_s9           /* flag to discard bootloader remnant int */

/* controller iobuf b_flags */

#define	B_NOCTL	040000		/* no bprobe response or timed-out */

/* unit	iobuf */

#define	Umask	io_s1		/* regB/regD device mask bit */
#define	Eca	io_s2		/* ptr to unit eca */
#define	Tpc	io_s3		/* tracks per cylinder */
#define	Cpv	io_s4		/* cylinders per volume */
#define	Statp	io_s5		/* i/o statistics block ptr */
#define	Badep	io_s6		/* bad track "ep" value */
/*              io_s7 Not used  */
#define Acts    io_s8           /* used for sorting queue */
#define Qcnt	io_s9		/* queue count */

/* unit	iobuf b_flags */

#define	B_OINP	 040000		/* open/close-in-progress */
#define	B_NOCNFG 020000		/* no format/config */

/* request buf */

#define	Qnext	av_forw		/* next on request queue */
#define	Soff 	av_back		/* sector offset from cylinder */
#define	Cylin	b_umd		/* cylinder offset */
#define	Block	b_resid		/* unsliced block number */

/* request buf b_flags */

#define B_CONF	0400000		/* configure command */ 
#define B_FMTV	0200000		/* format volume command */
#define B_FMTT	0100000		/* format track command */

#define	B_FINAL	040000		/* last i/o for request */
#define	B_SPEC	020000		/* special function (Command valid) */
#define	B_ABS	010000		/* do not unslice b_blkno */

/* flags and constants involved	with recalibration */

#define	POSERR	 0x0080		/* a positioning error */
#define	RECALCNT 3		/* number of times to retry a recalibrate */
#define	RETMSK	 1		/* indicates status for which it is not   */
				/* unreasonable to perform a retry */
 
/* identify all read/write I/O commands for correct I/O time accounting */
#define NON_RWIO B_SPEC

/* Macro to generate block offset of I/O for error logging */
#define IO_BOFF 	bp->Block

/* Needed for elevator algorithm */
#define STEPSIZE	32
/********************************************************************
  externals (in	config module)
*/

extern int	m320_cnt;  /* total logical devices (ctls * units * slices) */
extern int	m320_ilev[];   /* interrupt level */
extern int	m320_ivec [];  /* interrupt address , NOT number */
extern struct m3csr *m320_addr [];	     	/* controller csr addrs */
extern struct size m320_sizes [MAXUNIT][M320SLICE];
extern struct iobuf m320tab [];
extern struct iobuf m320utab [][MAXUNIT];
extern struct iotime m320stat [][MAXUNIT];
extern struct m320eregs m320eregs[];
extern int	m320otbl[];		/* open table */

/********************************************************************
  event	control	areas (eca)
*/

#define	FMTFILL	0		/* format fill byte, 0xE5 is standard */

/*
  "Default" hard-disk eca:
    sector length = 256
    heads = 1
    sectors/track = 32
    low-write-current and precomp = 2
  Default cylinders (not in eca) set = 1 by m320init
  Heads	and cylinders are set from on-disk config data by m320open
  Unit iobuf information is derived from the eca
*/

#define	STEPR	10

#define	HECA {0,0,0,10,0,0,0x4,0,0,0,0,0,0,0,{0,0,0,0,0},\
	      0,0,0,0,0xF,1,1,FMTFILL,{0,0,0},2,1,32,STEPR,0,0,\
	      2,0,0,2,{0,0,0},{0,0,0},{0,0},{0,0,0,0,0,0},0}

/*
  "Standard" floppy-disk eca:
    sector length = 256
    heads = 2
    sectors/track = 16
    low-write-current and precomp = 40
  Cylinders (not in eca) set = 80 by m320init
  Unit iobuf information is derived from the eca
  This setup is	for double-density tracks. Track 0 / other track
  differences are handled dynamically by invocation of m3eca0/m3ecaN.
*/

#define	FECA {0,0,0,10,0,0,0x4,0,0,0,0,0,0,0,{0,0,0,0,0},\
	      0,0,0,0,0x36,3,1,FMTFILL,{0,0,0},5,2,16,0xC,0x46,0x46,\
	      0,0,40,40,{0,0,0},{0,0,0},{0,0},{0,0,0,0,0,0},0}

/*
  Controller eca's - two hard disks, two floppies
*/

#define	CECA {HECA, HECA, FECA,	FECA}

LOCAL struct m3e_eca ecatab [MAXCTL][MAXUNIT] = {CECA, CECA};

#define	ECP(x) {&ecatab[x][0],&ecatab[x][1],&ecatab[x][2],&ecatab[x][3]}

LOCAL struct m3e_eca *ecptab [MAXCTL][MAXUNIT] = {ECP(0), ECP(1)};
    
/*
  Unit pointer to request buffer
*/

#define  UNITP &m320utab[CTL(bp->b_dev)][UNIT(bp->b_dev)] 


/********************************************************************
  device parameters fixed by format
*/

/* hard	disk */
#define	HBPS	256		/* bytes per sector */
#define	HSPU	(BSIZE/HBPS)	/* sectors per UNIX block */
#define	HSPT	32		/* sectors per track */
#define	HBPT	(HSPT*HBPS)	/* bytes per track */

/* floppy */
#define	FBPS	256		/* bytes per sector */
#define	FSPU	(BSIZE/FBPS)	/* sectors per UNIX block */
#define	FSPT	16		/* sectors per track */
#define	FSPT0	8		/* "logical" sectors on track 0 */
#define	FBPT	(FSPT*FBPS)	/* bytes per track */

/********************************************************************
  IOCTL	command	codes
*/

#define	M320FMTT  1	/* format track special function */
#define	M320FMTV  2	/* format volume ... */
#define M320CHGBT 3000	/* replace in core bad track table with new
			   bad track table on disk */
#define	M320CNFG  20	/* configure hard disk (not defined externally) */

/********************************************************************
  Naming conventions

  A note on naming conventions -- All defined names which overlay storage
  in the UNIX buf and iobuf structures are spelled with	an initial capital
  letter; e.g. "Request". The following	pointer	variable names are reserved
  for the indicated usage:

    cp -- a pointer to an iobuf	corresponding to a controller
    up -- a pointer to an iobuf	corresponding to a unit	(device)
    bp -- a pointer to a buf structure representing a request
    sp -- a pointer to an entry	(base/size) in a slice table
    ecp	-- a pointer to	a unit event control area (eca)
    iop	-- the base address of a controller's command and status registers

*/

/*********************************************************************
  Data structure usage and interconnections

  Every	controller is represented by an	iobuf (member of the array
  m320_tab). 
  The currently	active request buf is pointed to by cp->Request	and is not
  on a queue.  Context information for the currently active request, such
  as bytes remaining for a segmented i/o, is maintained	in the controller
  iobuf.

  Every	unit (device) is represented by	an iobuf (member of the	array
  m320_utab). The structure is used as a repository for	device-unique
  information, such as total cylinders on the volume, and as the interface
  to UNIX error-logging. When a	request	buf is queued bp->Unitp	is set
  to the address of the	associated unit	iobuf for future access	to the
  device-unique	information. No	queues from the	unit iobuf are maintained.

  8/19/86 - baw
  Got rid of Unitp pointer, needed space for sector offset.
  Replaced by UNITP macro definition.

  Every	buf represents a request to the	controller which will require one
  or more controller-level i/o operations. Except for initial/transition
  states every buf is either on	a queue	from the controller iobuf or
  pointed to by	cp->Request. The buf structure contains	a small	amount of
  overlay-field	information, such as the special function command code.

*/

/********************************************************************
  Request scheduling

  Control flow

  Requests enter the scheduling	process	when a buf is passed to
  m320_strategy. This routine performs some parameter validation, and then
  examines the B_PHYS flag bit in the buf. Requests which have this attribute
  are (usually)	swaps or come from memory-locked processes. The	bit test
  selects one of two request queues on the controller iobuf for	the
  request. Requests are	FIFO-queued with no attempt to arrange them by
  device or arm	position. If the controller is not currently busy
  m3start is then invoked.

  m3start examines the controller-busy flag. If	it is on (which	can happen
  only when m3start is invoked by the interrupt	handler	- see below) then
  the current request pointed to by cp->Request	requires additional service
  and remains current. Otherwise a new request is fetched from one one of
  the two controller queues giving preference to B_PHYS	requests. The
  selected request (if there is	one) is	passed to a handler routine determined
  by the type of request.

  The handler routines - m3hrw,	m3hspec, m3frw and m3fspec - all have the
  same general logical structure: If B_ERROR in	the buf	flags (again only
  from the interrupt handler) is set the current operation is an error
  retry. Otherwise a first-time	flag (actually cp->Request) is examined.
  The request context information in the controller iobuf is then either
  initialized or updated. The physical operation is setup and started,
  concluding direct control flow for the request.

  On completion	of a controller	operation the interrupt	handler	is invoked.
  If a request must be terminated (normal completion or	too many errors)
  the waiting requestor	is awakened and	the controller-busy flag is
  cleared allowing m3start to fetch a new request. Otherwise (additional
  service required or error retry) the busy flag is left set. m3start
  is then invoked.

  Significant variables

  cp->Busy -- The controller-busy flag.	When non-zero cp->Request is valid.
  m3start interprets the flag "set" to mean that the current request requires
  additional service.

  cp->Request -- The address of	the current request buf. This field is
  cleared for the handlers as a	first-time flag	for context maintenance.

  B_FINAL in bp->b_flags -- This flag bit is set by a handler to tell the
  interrupt handler that no more service is needed (assuming no	error).

  B_ERROR in bp->b_flags -- The	i/o error flag bit. Handlers interpret this
  flag to mean that an error retry is requested.

*/

/********************************************************************
  Asynchronous processing

  (Note: In the	following discussion "priority"	refers to processOR
  - status register - priority and not to UNIX process priority.)

  Operations in	various	portions of the	driver must be protected from
  asynchronous events to prevent corruption of data, state flags, etc.
  The scheme used in this driver is based on the following understanding
  of the UNIX interrupt	and preemptive dispatch	mechanisms:

    Any	code section may lose control at any time to an	interrupt of
    higher priority. A timer interrupt may determine that the "current
    process" has reached the end of its	time slice. If so, the process
    is so marked but no	other action is	taken then.

    UNIX uses a	two-level interrupt processing scheme in which a
    first-level	interrupt handler "calls" a configured second-
    level handler (e.g.	m320intr). On return from a second-level
    handler, the timeout marker	is examined. If	the current process
    has	timed out AND the priority of the interrupted code section
    (at	top of stack) is ZERO, then and	only then will the current
    process be preempted.

    In consequence

    -  A code section running as a process may be interrupted,
       but if its priority is greater than zero	it will	complete
       without preemption (particularly	by other copies	of itself).

    -  A code section running as an interrupt handler may be
       interrupted itself by higher-priority interrupts, but will
       complete	without	preemption.

  This driver has four code sections which must	be protected from
  asynchronous interaction:

    - Getting exclusive	control	of a unit for open/close. This uses
      a	conventional sleep/spin	loop and need not be discussed
      further.

    - A	request	through	"strategy" to "start". This code runs as
      a	process.

    - A	controller interrupt.

    - A	watchdog timer event. This code	runs as	an interrupt, but
      (due to the pending implementation of the	"split timer"
      facility)	its priority is	unknown.

  Strategy code	is protected both from itself (another process)	and
  from controller interrupts by	bracketing critical code at
  controller interrupt priority. It is protected from higher-priority
  timer	events by a semaphore ("timelock") which tells the watchdog
  routine to skip the current cycle and	try again later.

  A controller interrupt is protected from strategy calls by the fact
  that no preemption can occur.	Higher-priority	timer events are
  blocked by the "timelock" semaphore.

  Note that timelock cannot be altered until interrupts	are blocked,
  and that it must be set before B_NOCTL (the watchdog "dead controller"
  flag)	can be examined.

  If a timer event runs	at less	than controller	interrupt priority
  it is	protected by raising its priority to maximum. This may
  result in a lengthy disabled period while queues are being flushed,
  but in that case the greater disaster	of a dead controller has
  already occurred.

  All calls to change priority in the driver are flagged with a	comment
  containing the string	"INTERRUPT LEVEL" for easy location.

*/

/********************************************************************
  Limited open and volume formatting

  When open is unable to read a	hard-disk configuration	area (or its
  contents are invalid)	flag bit B_NOCNFG is set in up->b_flags.
  A test in m320strategy prohibits (with EIO) all operations on	a
  disk in this state except (the mnemonics) M320FMTV (format volume)
  and M320CNFG (write configuration data). B_NOCNFG can	only be
  cleared by a close of	the physical device.

  When ioctl receives a	format volume request it expects valid device
  size parameters which	are installed in the appropriate places	in
  the unit iobuf and the eca. This permits correct size	and seek
  calculations in the format. After a volume format is complete
  M320CNFG is executed to write	the config data	to disk	block 0.
  Note that this function is not accessible to the user	through
  ioctl.

  A full open reads configuration data from the	disk itself, making
  the device driver independent	of disk	size except for	a one-time
  format call.

*/

/********************************************************************
  Sector numbering

  Hard-disk sectors are	numbered from 1	rather than the	common industry
  practice of 0. This is necessary since the controller	firmware (appears
  to) search for sector	1 when doing a track step.

  Floppy-disk sectors are numbered from	1 as is	customary.

  In both cases	the adjustment is made in the assignments to
  header.sect and ecp->m3e_sect	with zero-based	numbering used for
  all seek calculations.

*/

/********************************************************************
  Disk sector interleave

  Disk sector interleaving is provided only to prevent losing revolutions
  between the sectors of a UNIX	block transfer.	For this purpose the
  interleave factor may	be treated as a	hardware parameter and need not
  be user-settable. The	interleave table(s) are	static local data in the
  format routine(s).

  Hard-disks are interleaved. Floppies are not.

*/

/********************************************************************
  Track	zero handling

  All floppy disk tracks are MFM except	track 0. MFM tracks have FSPT
  sectors of FBPS bytes. Track zero has	FSPT sectors of	one-half FBPS
  bytes. FSPT0 is the number of	logical	(FBPS-byte) sectors on track
  zero.	(Eight-inch floppies might leave some track 0 sectors mapped
  out.)	Adjustment is made by incrementing the sector number to	offset
  the "missing"	sectors	to track 1.

*/

/********************************************************************
  Actual contoller count - computed from config-created	variable
  m320_cnt
*/

LOCAL	int	actctl;

/********************************************************************
  Watchdog timer semaphore -- when found set the timer skips a tick
  to protect iobufs and	queues
*/

LOCAL	int	timelock = 0;

/********************************************************************
  initialization routine
*/

m320init ()
{
	extern int m3timer ();
	register struct iobuf *cp, *up;
	register struct m3e_eca *ecp;
	register ctl, eptr, r, unit;
	register waitc;
	struct m3csr *iop;
	int	delta, c0, c1;


#ifdef DEBUG
	if(M320Debug & DBINIT)
		printf("M320init\n");
#endif
	/* assumes controller reset via	hardware, recalibrate used for */
	/* reset command */

	/* compute actual controller count */
	actctl = m320_cnt / (MAXUNIT * M320SLICE);

	/* (address mapping problem) verify address mapping */
	for (c1 = 0; c1	< actctl; ++c1)
		for (c0	 = 0; c0 < c1; ++c0) {
			if ((delta = (char *)m320_addr[c1] -
					(char *)m320_addr[c0]) < 0)
				delta = -delta;
			/* min distance between controller csr spaces */
			if (delta < 1024) {
				m320tab[c0].b_flags |= B_NOCTL;
				m320tab[c1].b_flags |= B_NOCTL;
				printf ("M320: controllers %d-%d, csr address conflict\n", c0, c1);
			}
		}

	/* controllers loop */
	for (ctl = 0, cp = m320tab; ctl < actctl; ++cp, ++ctl) {
		/* essential fields */
#ifdef DEBUGERR
/* clear io_nreg field to start, use for counting errors */
	cp->io_nreg =0;
#endif
		cp->Busy = 0;
		cp->Ioaddr = (int)(iop = m320_addr[ctl]);

		/* see if something there, 0 write should clear	pending	int */
		if ( cp->b_flags & B_NOCTL || bprobe (&iop->regD, 0)) {
			cp->b_flags |= B_NOCTL;
			continue;
		}

		/* init	csr */
		eptr = ((int)ecptab[ctl]) >> 1;
		for (r = 1; r < 8; r += 2) {
			*(((ubyte * )iop) + r) = (ubyte)eptr;
			eptr >>= 8;
		}

		/* units loop */
		for (unit = 0; unit < MAXUNIT; ++unit) {
			up = &m320utab[ctl][unit];
			/* io_addr must	be non-0 for error log,
			   dummy value ok if io_nreg == 0 */
			up->io_addr = (paddr_t)&m320eregs[ctl];
			up->io_nreg = M320EREG;
			up->Umask = 0x10 << unit;
			up->Eca	 = (int)(ecp = ecptab[ctl][unit]);
			up->Statp = (int)(&m320stat[ctl][unit]);
			up->Badep = NULL;  /* safety first */
			/* default config for hard disk,
				standard for floppies */
			up->Tpc = ecp->m3e_nhed;
			up->Cpv = (FLOPPY(MKDEV(ctl, unit, 0))) ? 80 : 1;

			if ( !(FLOPPY(MKDEV(ctl, unit, 0))) ) {
				/* issue a recalibrate command */
				waitc = 0x100000;
				ecp->m3e_cmd = M3C_CALB;
				ecp->m3e_mnst = M3S_BUSY;
				ecp->m3e_exst = 0;
				iop->reg9 = 0;
				iop->regB = 0;
				/*  start i/o, wait for	completion or timeout */
				iop->regD = up->Umask;
				while((ecp->m3e_mnst == M3S_BUSY) && (--waitc));
				iop->regD = 0;	/* clear interrupt */
				/*
				 * Ignore error at this point - can't
				 * distinguish between a real error and no
				 * drive. This is only included for those
				 * second hard drives that need a "reset".
				 * It won't be done anywhere else.
				 */
			}

		}  /* end units	*/
		iop->reg9 = m320_ivec[ctl]/4;	 /* address -> int number */
	}  /* end controllers */

	/* start timeout routine */
	timeout (m3timer, 0, 100*HZ);  /* can't call m3timer since it spl's */

#ifdef DEBUG
	if(M320Debug & DBINIT)
		printf("M320init exit\n");
#endif
}


/********************************************************************
  open
*/

m320bopen (dev,flag,otyp)	 /* block device open */
register dev,flag,otyp;
{
#ifdef DEBUG
	if(M320Debug & DBBOPEN)
		m320print(dev,"M320bopen");
#endif
	m3open (dev,flag,otyp,(1 << SLICE(dev)));  /* block dev slice bit */
#ifdef DEBUG
	if(M320Debug & DBBOPEN)
		printf("M320bopen exit\n");
#endif
}


m320copen (dev,flag,otyp)	 /* char special device	open */
register dev,flag,otyp;
{
#ifdef DEBUG
	if(M320Debug & DBCOPEN)
		m320print(dev,"M320copen");
#endif
	m3open (dev,flag,otyp,(1 << (SLICE(dev) )));  /* char dev slice bit */
#ifdef DEBUG
	if(M320Debug & DBCOPEN)
		printf("M320copen exit\n");
#endif
}


LOCAL m3open (dev,flag,otyp,mask)
register dev,flag,otyp,mask;
{
	register struct iobuf *cp, *up;
	register struct buf *bp;
	register struct m3e_eca *ecp;
	struct volumeid *vp;
	struct dkconfig *cfp;
	register	ctl, unit, xbad;
	int	*otbl;		/* address of open table */

	/* validate */
	cp = &m320tab[(ctl = CTL(dev))];
	up = &m320utab[ctl][(unit = UNIT(dev))];
	otbl=&m320otbl[((ctl * MAXUNIT) + unit) * OTYPCNT];
	if (ctl >= actctl || ILLEGAL(dev) || cp->b_flags & B_NOCTL) {
		u.u_error = ENXIO;
		return;
	}

	/* floppies do not have	a config block */
	if (FLOPPY(dev))
		return;

	/* get exclusive control of unit */
	m3excl (up);

	/* mark	logical	device open */
	if(otyp == OTYP_LYR)
		++otbl[OTYP_LYR];
	else if( otyp < OTYPCNT)
		otbl[otyp] |= mask;

	/* if physical device already open, release unit and done */
	if (up->b_flags & B_OPEN) {
		m3rxcl (up);
		return;
	}

	/* mark	physical device	open */
	up->b_flags |= B_OPEN;

	/* get a buffer	for the	config data */
	bp = geteblk ();
	bp->b_flags  |= B_PHYS | B_ABS | B_READ;
	bp->b_error = 0;
	bp->b_proc = u.u_procp;
	bp->b_dev = dev;
	bp->Block = 0;	/* reading absolute block zero */

	/* setup ptrs assuming vid is sector 0,	config area is sector 1	*/
	vp = (struct volumeid *)(bp->b_un.b_addr);
	cfp = (struct dkconfig *)((int)bp->b_un.b_addr + 256);

	/* send	to strategy */
	m320strategy (bp);
	iowait (bp);

	/* error check,	signature string */
	if (bp->b_flags & B_ERROR || !m3sig (vp->vid_mac)) {
		up->b_flags |= B_NOCNFG;
		goto done;
	}

	/* get/set config data */
	ecp = (struct m3e_eca *)(up->Eca);
	up->Tpc	 = ecp->m3e_nhed = cfp->ios_hds;
	up->Cpv	 = cfp->ios_trk;
	ecp->m3e_lowr = ecp->m3e_pcmp = cfp->ios_pcom;

	/* get bad track count,	skip if	zero */
	if ((xbad = vp->vid_btl) == 0)
		goto done;

	/* ensure that all bad track data fits in a buffer */
	if (HASHSIZE + xbad * sizeof(struct badent ) > SBUFSIZE) {
		/* geteblk init */
		printf ("M320: drive %d, ctl %d, bad track data	exceeds	%d buffer\n", unit, ctl, SBUFSIZE);
		up->b_flags |= B_NOCNFG;
		goto done;
	}

	/* now read absolute block 1 */
	brelse (bp);
	bp = geteblk ();
	bp->b_flags |= B_PHYS | B_ABS | B_READ;
	bp->b_error = 0;
	bp->b_proc = u.u_procp;
	bp->b_dev = dev;
	bp->Block = 1;

	/* send	to strategy */
	m320strategy (bp);
	iowait (bp);

	/* error check,	bad track initialization */
	if (bp->b_flags	& B_ERROR
	|| (up->Badep = badinit (bp->b_un.b_addr,xbad)) == NULL) {
		up->b_flags |= B_NOCNFG;
	}

done:
	brelse (bp);

	/* clear leftovers from	iowait */
	u.u_error = 0;

	/* note	config failure */
	if (up->b_flags	 & B_NOCNFG)
		printf ("M320: drive %d, ctl %d, open found no format/config\n",
		     		    unit, ctl);

	/* release control of device */
	m3rxcl (up);
}


/**********************************************************************
  close
*/

m320bclose (dev,flag,otyp)	/* block dev close */
register dev,flag,otyp;
{
#ifdef DEBUG
	if(M320Debug & DBBCLOSE)
		m320print(dev,"M320bclose");
#endif
	m3close	(dev,flag,otyp,(1 << SLICE(dev)));  /* slice mask bit */
}


m320cclose (dev,flag,otyp)	/* char	special	dev close */
register dev,flag,otyp;
{
#ifdef DEBUG
	if(M320Debug & DBCCLOSE)
		m320print(dev,"M320cclose");
#endif
	m3close	(dev,flag,otyp,(1 << (SLICE(dev) )));	/* slice mask bit */
}


LOCAL m3close (dev,flag,otyp,mask)
register	dev,flag,otyp,mask;
{
	register struct iobuf *cp, *up;
	register	ctl, unit, i;
	register int	*otbl;		/* address of open table */ 

	/* validate */
	cp = &m320tab[(ctl = CTL(dev))];
	up = &m320utab[ctl][(unit = UNIT(dev))];
	otbl=&m320otbl[((ctl * MAXUNIT) + unit) * OTYPCNT];
	if (ctl	 >= actctl || ILLEGAL(dev) || cp->b_flags & B_NOCTL) {
		u.u_error = ENXIO;
		return;
	}

	/* mark logical device closed */
	if(otyp == OTYP_LYR)
		--otbl[OTYP_LYR];
	else if (otyp < OTYPCNT)
		otbl[otyp] &= ~mask;

	/* no more action on floppies */
	if (FLOPPY(dev))
		return;

	/* get the unit	*/
	m3excl (up);

	/* close physical device when all logical devices are closed */
	for( i = 0; i < OTYPCNT && otbl[i] == 0; ++i)
		;

	if( i == OTYPCNT){
		/* B_OPEN and B_NOCNFG off */
		up->b_flags &= ~(B_OPEN | B_NOCNFG);

		/* kill	bad track table	*/
		baddel ((char *)up->Badep);
		up->Badep = NULL;
	}

	/* release the unit */
	m3rxcl (up);

}


/*********************************************************************
  open/close support
*/

LOCAL m3excl (up)  /* get exclusive control of unit */
register struct iobuf *up;
{
	/* sleep/spin on in-progress flag */
	spl6 ();  /* INTERRUPT LEVEL */
	while (up->b_flags & B_OINP)
		sleep (up, PRIBIO);

	/* now grab it */
	up->b_flags |= B_OINP;
	spl0 ();  /* INTERRUPT LEVEL */

}


LOCAL m3rxcl (up)  /* release exclusive	control	of unit	*/
register struct iobuf *up;
{
	/* clear flag, wakeup any waiting */
	up->b_flags &= ~B_OINP;
	wakeup (up);
}


LOCAL int m3sig	(str)  /* validate signature string in VID */
register char	*str;
{
	static char *valid[] = { "MOTOROLA", "EXORMACS", NULL };
	register char	**x, *y, *z;

	for (x = valid; *x != NULL; ++x) {
		for (y = str, z	 = *x; *z != '\0'; ++z, ++y)
			if (*y != *z)
				break;
		if (*z == '\0')
			return 1;
	}
	return 0;
}


/********************************************************************
  strategy routine
*/

m320strategy (bp)
register struct buf *bp;
{
	register struct iobuf *cp, *up;
	register struct iotime *ip;
	struct size *sp;
	register   ctl, unit, upper;
	register   xsect;

#ifdef DEBUG
	if(M320Debug & DBSTRAT) {
		m320print(bp->b_dev,"M320strategy");
		printf("blkno=%x bcount=%x flags=%x\n", bp->b_blkno,
			bp->b_bcount, bp->b_flags);
	}
#endif
	/* derive controller, unit */
	cp = &m320tab[(ctl = CTL(bp->b_dev))];
	up = &m320utab[ctl][(unit = UNIT(bp->b_dev))];
	sp = &m320_sizes[unit][SLICE(bp->b_dev)];
	if (ctl	 >= actctl || ILLEGAL(bp->b_dev)) {
		bp->b_flags |= B_ERROR;
		bp->b_error = ENXIO;
		bp->b_resid = bp->b_bcount;
		m3eio (bp);
		return;
	}

	/* validate configured drive */
	if (up->b_flags	 & B_NOCNFG)
		/* volume format commands ok */
		/* others are not */
		if (!(bp->b_flags & B_SPEC) || ((!(bp->b_flags & B_FMTV)&&
		    !(bp->b_flags & B_CONF)))) {
			bp->b_flags |= B_ERROR;
			bp->b_error = EIO;
			bp->b_resid = bp->b_bcount;
			m3eio (bp);
			return;
		}


	/* translate block number if not abs request */
	if (!(bp->b_flags & B_ABS)) {
		upper = sp->nblocks;
		if (bp->b_blkno	 < 0 || bp->b_blkno >= upper) {
			if (bp->b_blkno	== upper && (bp->b_flags & B_READ))
				bp->b_resid = bp->b_bcount;
			else {
				bp->b_flags |= B_ERROR;
				bp->b_error = ENXIO;
			}
			m3eio (bp);
			return;
		}
		bp->Block = bp->b_blkno	 + sp->blkoff;
	}

	/* start request time */
	bp->b_start = lbolt;

	/* initialize forward pointer on buffer */
	bp->Qnext = NULL;

	/* compute cylinder and sector offset */
	if ( FLOPPY(bp->b_dev) ) {

		xsect = bp->Block*FSPU;
		if ( xsect < FSPT0 ) { /* starting sector in first track */
			bp->Cylin = 0;
			*(int *)(&bp->Soff) = xsect;
		} else { /* starting sector beyond first track */
			bp->Cylin = (xsect + FSPT - FSPT0) / (FSPT * up->Tpc);
			*(int *)(&bp->Soff) = (xsect + FSPT - FSPT0) % (FSPT * up->Tpc);
		}
	} else { /* hard disk */ 

		bp->Cylin = (bp->Block * HSPU) / (HSPT * up->Tpc);
		*(int *)(& bp->Soff) = (bp->Block * HSPU) % (HSPT * up->Tpc);

	}

	/* log I/O statistics for this drive */
	ip = &m320stat[ctl][unit];
	if ( (bp->b_flags & NON_RWIO ) )
		++(ip->io_other);
	else {
		++(ip->io_cnt);
		ip->io_bcnt += btoc(bp->b_bcount);
	}

	splx(SPLEV(m320_ilev[ctl]));  /* INTERRUPT LEVEL */
	timelock = 1;

	/* finish validation, must be disabled because of timeout */
	if (cp->b_flags	 & B_NOCTL) {
		bp->b_flags |= B_ERROR;
		bp->b_error = ENXIO;
		m3eio (bp);
		timelock = 0;
		spl0 ();
		return;
	}

	/* enqueue request */

	if (up->b_actf == NULL) {

		up->b_actf = bp;
		up->b_actl = bp;
		up->Acts = (int) bp;

	} else {

		register struct buf *ap;
		register struct buf *fp;

		if ( ( (ip->io_cnt) & (STEPSIZE - 1) ) == 0 )
			up->Acts = (int)up->b_actl;

		for ( ap = (struct buf *)up->Acts; fp = ap->Qnext; ap=fp) {

			int s1,s2;
	
			if ( (s1 = ap->Cylin - bp->Cylin ) < 0 ) s1 = -s1;
			if ( (s2 = ap->Cylin - fp->Cylin ) < 0 ) s2 = -s2;
			if (s1 < s2)
				break;

		}

		ap->Qnext = bp;
		if ( (bp->Qnext = fp) == NULL )
			up->b_actl = bp;

	}

 	up->Qcnt++;

	/* still disabled, try a start */
	if (!cp->Busy)
		m3start	(ctl);

	timelock = 0;
	spl0 ();  /* INTERRUPT LEVEL */

}


/********************************************************************
  start	i/o routine
*/

LOCAL m3start (ctl)
register ctl;
{
	register struct iobuf *cp;
	register struct buf *bp;
        register struct iobuf *up;
	register struct iobuf *nextup;
	static int count = 0;
	static int d;

	/* this	routine	must run at controller interrupt priority */
	cp = &m320tab[ctl];

	/* if the busy flag is on the last request remains current */
	if (cp->Busy)
		bp = (struct buf *)cp->Request;

		/* otherwise fetch from	the queue */                             
	else {

		/* select next device to be serviced */
		cp = &m320tab[ctl];
		up = (struct iobuf *)NULL;
		nextup = &m320utab[ctl][0];

		for ( d=0; d<MAXUNIT; d++,nextup++) {

			if (nextup->b_actf != NULL) {

				up = nextup;
				break;

			}
		}

		if (up == NULL)
			return;

		bp = up->b_actf;

		cp->Request = NULL;		/* first-time flag */
		cp->b_errcnt = 5;  		/* error (attempts) count */
	}

	/* disentangle major cases and pass to handler */
	if (FLOPPY(bp->b_dev)) {
		if (bp->b_flags & B_SPEC)
			m3fspec	(cp, bp);
		else
			m3frw (cp, bp);

	} else if (bp->b_flags & B_SPEC)
		m3hspec	(cp, bp);
	else
		m3hrw (cp, bp);

}


/********************************************************************
  hard-disk read/write
*/

LOCAL m3hrw (cp, bp)
register struct iobuf *cp;
register struct buf *bp;
{
	register struct m3e_eca *ecp;
	register struct iobuf *up;
	register 	xsect, xtrack, xcyl, xtcnt;
	int	xalt;
#ifdef DEBUGQ    
	int  mult;
	int  alt;
	mult = 0;
	alt = 0;  
#endif

	/* address unit, eca */
	ecp = (struct m3e_eca *)(up = UNITP)->Eca;

	/* if error, turn off flag and prepare to re-issue command */
	if (bp->b_flags	 & B_ERROR)
		bp->b_flags &= ~B_ERROR;

	/* else	if first time, setup context in	controller iobuf */
	else if (cp->Request == NULL) {
		cp->Request = (int)bp;
		cp->Sector = bp->Block * HSPU;
		cp->Buffer = (int)bp->b_un.b_addr;
		cp->Count = bp->b_bcount;

	/* otherwise, multiple-i/o request, update context */
	} else {
#ifdef DEBUGQ
	        mult = 1;
#endif
		cp->Sector += cp->Rcount / HBPS;
		cp->Buffer += cp->Rcount;
		cp->Count -= cp->Rcount;
	}

	/* now setup rest of eca */
	ecp->m3e_cmd = (bp->b_flags & B_READ) ? M3C_REMS : M3C_WRMS;
	ecp->m3e_bufa = cp->Buffer;
	xtrack = cp->Sector / HSPT;

	/* same sector regardless of alt */
	xsect = cp->Sector - xtrack * HSPT;
	ecp->m3e_sect = xsect + 1;
	/* compute track count */
	xtcnt = (cp->Sector + cp->Count / HBPS - 1) / HSPT - xtrack + 1;
	if ((cp->Rcount = badchk (up->Badep, xtrack, xtcnt, &xalt)) == 0) {
		xtrack = xalt;
#ifdef DEBUGQ
		alt = 1;
#endif
		/* space to end	of alt track */
		cp->Rcount = (HSPT - xsect) * HBPS;
	} else {
		/* space to next bad track */
		cp->Rcount *= HSPT;		/* convert to sector count */
		cp->Rcount -= xsect;		/* don't go off	last track */
		cp->Rcount *= HBPS;		/* convert to bytes */
	}
	if (cp->Rcount > cp->Count)
		cp->Rcount = cp->Count;
	xcyl = xtrack / up->Tpc;
#ifdef DEBUGQ
	if ( !(xcyl == bp->Cylin) && (mult == 0) && (alt == 0) ) {
		printf ("Hard disk: xcyl = %x  Cylin = %x\n",xcyl,bp->Cylin);
		printf ("Hard disk: Block = %x  Tpc = %x \n",bp->Block,up->Tpc);
 		printf ("Hard disk: xtrack = %x\n",xtrack);
	}
#endif
	ecp->m3e_head = xtrack - xcyl * up->Tpc;
	ecp->m3e_cyl = xcyl;
	if (cp->Rcount > MAXXFER)
		cp->Rcount = MAXXFER;
	ecp->m3e_bufl = cp->Rcount;

	/* if final request, set flag for int handler */
	if (cp->Count == cp->Rcount)
		bp->b_flags |= B_FINAL;

	/* start the i/o */
	m3sio (cp);

}


/********************************************************************
  hard-disk special functions routine
*/

LOCAL m3hspec (cp, bp)
register struct iobuf *cp;
register struct buf *bp;
{
	/* the hard disk interleave table */
	static char ilvtab [HSPT] = 
	 {
		1, 12, 23, 2, 13, 24, 3, 14, 25, 4, 15, 26, 5, 16, 27, 6,
		17, 28, 7, 18, 29, 8, 19, 30, 9, 20, 31, 10, 21, 32, 11, 22
	};

	/* hard disk sector header format data */
	register struct header {
		ubyte hdr_cyl1;	   /* cylinder high byte */
		ubyte hdr_cyl0;	   /* cylinder low byte	*/
		ubyte hdr_head;	   /* head */
		ubyte hdr_sect;	   /* sector */
		ubyte hdr_slc;	   /* sector length code */
	} *hp;
	register struct m3e_eca *ecp;
	register struct iobuf *up;
	register struct volumeid *vp;
	register struct dkconfig *cfp;
	register 	xcyl, sn;
	register char	*p0, *p1;


	/* address unit, eca */
	ecp = (struct m3e_eca *)(up = UNITP)->Eca;

	/* by command ... */

	if (bp->b_flags & B_FMTT || bp->b_flags & B_FMTV ) {
		/* if error retry, clear flag and adjust count */
		if (bp->b_flags	 & B_ERROR) {
			bp->b_flags &= ~B_ERROR;
			++cp->Count;

		/* if first time, setup	context	*/
		} else if (cp->Request == NULL) {
			cp->Request = (int)bp;
			if (bp->b_flags & B_CONF ) {
				cp->Track = (bp->Block * HSPU) / HSPT;
				cp->Count = 1;
			} else
			 {
				cp->Track = 0;
				cp->Count = up->Cpv * up->Tpc;
			}

		/* else	update context */
		} else
			++cp->Track;

		/* setup eca */
		ecp->m3e_cmd = M3C_FORM;
		ecp->m3e_bufa = (uint32)bp->b_un.b_addr;
		ecp->m3e_bufl = HSPT * 5;
		ecp->m3e_sect = 0;
		xcyl = cp->Track / up->Tpc;
		ecp->m3e_head = cp->Track - xcyl * up->Tpc;
		ecp->m3e_cyl = xcyl;

		/* build sector	headers	in buffer */
		for (sn = 0, hp = (struct header *)bp->b_un.b_addr;  sn < HSPT;
				hp = (struct header *)((int)hp + 5), ++sn) {
			hp->hdr_cyl1 = (ecp->m3e_cyl) >> 8;
			hp->hdr_cyl0 = (ecp->m3e_cyl) & 	0xFF;
			hp->hdr_head = ecp->m3e_head;
			hp->hdr_sect = ilvtab[sn];
			hp->hdr_slc = ecp->m3e_slc;
		}

		/* if last track set switch */
		if (--cp->Count	 == 0)
			bp->b_flags |= B_FINAL;

		/* start i/o */
		m3sio (cp);

	/* end of format */
	} else if (bp->b_flags & B_CONF) {
		/* any repeat was an error */
		bp->b_flags &= ~B_ERROR;

		/* first time */
		cp->Request = (int)bp;

		/* setup eca */
		ecp->m3e_cmd = M3C_WRMS;
		ecp->m3e_bufa = (uint32)bp->b_un.b_addr;
		ecp->m3e_bufl = BSIZE;
		ecp->m3e_sect = 1;
		ecp->m3e_head = 0;
		ecp->m3e_cyl = 0;

		/* address buffer as vid and config area */
		vp = (struct volumeid *)bp->b_un.b_addr;
		cfp = (struct dkconfig *)((int)bp->b_un.b_addr + 256);

		/* fill	zeros, essential values	*/
		for (p0 = (char *)vp, p1 = (char *)((int)vp + 512); p0<p1; ++p0)
			*p0 = 0;
		vp->vid_btl = 0;
		cfp->ios_hds = up->Tpc;
		cfp->ios_trk = up->Cpv;
		for (p0	 = "MOTOROLA", p1 = (char *)vp->vid_mac; *p0 != '\0';
		    ++p1, ++p0)
			*p1 = *p0;

		/* this	is final request */
		bp->b_flags |= B_FINAL;

		up->b_flags &= ~B_NOCNFG;
		/* start i/o */
		m3sio (cp);
		/* end of config */ 

	/* bad command */	
	} else {
		printf ("M320: invalid special command (2) %d\n", bp->b_flags & (B_FMTT | B_FMTV | B_CONF ) );
		bp->b_flags |= B_ERROR;
		bp->b_error = EINVAL;
		m3eio (bp);
	}

}


/********************************************************************
  floppy-disk read/write routine
*/

LOCAL m3frw (cp, bp)
register struct iobuf *cp;
register struct buf *bp;
{
	register struct m3e_eca *ecp;
	register struct iobuf *up;
	register 	xsect, xtrack, xcyl;
#ifdef DEBUG
	int  mult;
	mult = 0;
#endif

	/* address unit, eca */
	ecp = (struct m3e_eca *)(up = UNITP)->Eca;

	/* if error, turn off flag and prepare to re-issue command  */
	if (bp->b_flags	 & B_ERROR)
		bp->b_flags &= ~B_ERROR;

		/* else	if first time, setup context in	controller iobuf */
	else if (cp->Request == NULL) {
		cp->Request = (int)bp;
		cp->Sector = bp->Block * FSPU;
		cp->Buffer = (int)bp->b_un.b_addr;
		cp->Count = bp->b_bcount;

	/* otherwise, multiple-i/o request, update context */
	} else {
#ifdef DEBUG
	        mult = 1;
#endif
		cp->Sector += cp->Rcount / FBPS;
		cp->Buffer += cp->Rcount;
		cp->Count -= cp->Rcount;
	}

	/* now setup eca */
	ecp->m3e_cmd = (bp->b_flags & B_READ) ? M3C_REMS : M3C_WRMS;
	ecp->m3e_bufa = cp->Buffer;
	/* track 0 */
	if (cp->Sector < FSPT0)	{
		if (cp->Count < (cp->Rcount = (FSPT0 - cp->Sector) * FBPS))
			cp->Rcount = cp->Count;
		ecp->m3e_sect = (cp->Sector << 1) + 1;
		ecp->m3e_head = 0;
		ecp->m3e_cyl = 0;
		m3eca0 (ecp);  /* eca format fields */

	} else {
		cp->Rcount = cp->Count;
		/* adjust "missing" trk 0 sectors */
		xsect = cp->Sector + FSPT - FSPT0;
		xtrack = xsect / FSPT;
		ecp->m3e_sect = xsect - xtrack * FSPT + 1;
		xcyl = xtrack / up->Tpc;
#ifdef DEBUG
	if ( !(xcyl == bp->Cylin) && (mult == 0))  {
		printf ("Floppy disk: xcyl = %x  Cylin = %x\n",xcyl,bp->Cylin);
	}
#endif
		ecp->m3e_head = xtrack - xcyl * up->Tpc;
		ecp->m3e_cyl = xcyl;
		m3ecaN (ecp);  /* eca format fields */
	}
	if (cp->Rcount > MAXXFER)
		cp->Rcount = MAXXFER;
	ecp->m3e_bufl = cp->Rcount;

	/* if final request, set flag for int handler */
	if (cp->Count == cp->Rcount)
		bp->b_flags |= B_FINAL;

	/* start the i/o */
	m3sio (cp);

}


/********************************************************************
  floppy-disk special functions	routine
*/

LOCAL m3fspec (cp, bp)
register struct iobuf *cp;
register struct buf *bp;
{
	/* floppy disk sector header format data */
	register struct header {
		ubyte hdr_cyl;	   /* cylinder */
		ubyte hdr_head;	   /* head */
		ubyte hdr_sect;	   /* sector */
		ubyte hdr_slc;	   /* sector length code */
	} *hp;
	register struct m3e_eca *ecp;
	register struct iobuf *up;
	register	xcyl, sn;


	/* address unit, eca */
	ecp = (struct m3e_eca *)(up = UNITP)->Eca;

	/* by command ... */

	if (bp->b_flags & B_FMTT || bp->b_flags & B_FMTV) {
		/* if error retry, clear flag and adjust count */
		if (bp->b_flags	 & B_ERROR) {
			bp->b_flags &= ~B_ERROR;
			++cp->Count;

		/* if first time, setup	context	*/
		} else if (cp->Request == NULL) {
			cp->Request = (int)bp;
			if (bp->b_flags & B_FMTT) {
				cp->Track = bp->Block * FSPU;
				if (cp->Track >= FSPT0)
					cp->Track += FSPT - FSPT0;
				cp->Track /= FSPT;
				cp->Count = 1;

			} else {
				cp->Track = 0;
				cp->Count = up->Cpv * up->Tpc;
			}

		/* else	update context */
		} else
			++cp->Track;

		/* setup eca */
		ecp->m3e_cmd = M3C_FORM;
		ecp->m3e_bufa = (uint32)bp->b_un.b_addr;
		ecp->m3e_bufl = FSPT * 4;
		ecp->m3e_sect = 0;
		xcyl = cp->Track / up->Tpc;
		ecp->m3e_head = cp->Track - xcyl * up->Tpc;
		ecp->m3e_cyl = xcyl;
		if (ecp->m3e_cyl == 0 && ecp->m3e_head == 0)
			m3eca0 (ecp);  /* track	0 format */
		else
			m3ecaN (ecp);
		/* NOTE	m3ecaX sets sector length code for buffer build	*/

		/* build sector	headers	in buffer */
		for (sn = 0, hp = (struct header *)bp->b_un.b_addr;  sn < FSPT;
				hp = (struct header *)((int)hp + 4), ++sn) {
			hp->hdr_cyl = ecp->m3e_cyl;
			hp->hdr_head = ecp->m3e_head;
			hp->hdr_sect = sn + 1;
			hp->hdr_slc = ecp->m3e_slc;
		}

		/* if last request, set	flag */
		if (--cp->Count	 == 0)
			bp->b_flags |= B_FINAL;

		/* start i/o */
		m3sio (cp);
		/* end of format */

	/* bad command */
	} else {
		printf ("M320: invalid special command (2) %d\n", bp->b_flags & (B_FMTT | B_FMTV | B_CONF ) );
		bp->b_flags |= B_ERROR;
		bp->b_error = EINVAL;
		m3eio (bp);
	}

}


/********************************************************************
  routines fill	track format parameters	in eca for floppies
*/

LOCAL m3eca0 (ecp)  /* for track 0 */
register struct m3e_eca *ecp;
{
	ecp->m3e_n4 = 0x1B;
	ecp->m3e_slc = 0;
	ecp->m3e_drvt = 4;
}


LOCAL m3ecaN (ecp)  /* for all other tracks */
register struct m3e_eca *ecp;
{
	ecp->m3e_n4 = 0x36;
	ecp->m3e_slc = 1;
	ecp->m3e_drvt = 5;
}


/********************************************************************
  print	routine
*/

m320print (dev, str)
register 	dev;
register char	*str;
{
	printf ("%s on M320 drive %d, ctl %d, slice %d\n", str, UNIT(dev),
	     CTL(dev), SLICE(dev));
}
/**************************************************************************
 Physical I/O Breakup routine
 */
m320breakup(bp)
register struct buf *bp;
{
	dma_breakup(m320strategy, bp, BSIZE);
}



/********************************************************************
  read/write
*/

m320read (dev)
register dev;
{
#ifdef DEBUG
	if(M320Debug & DBREAD)
		m320print(dev,"M320read");
#endif
	if (u.u_offset & BMASK || u.u_count & BMASK) {
		u.u_error = EINVAL;
		return;
	}
	if (physck (m320_sizes[UNIT(dev)][SLICE(dev)].nblocks, B_READ))
		physio (m320breakup, 0, dev, B_READ);
}


m320write (dev)
register 	dev;
{
#ifdef DEBUG
	if(M320Debug & DBWRITE)
		m320print(dev,"M320write");
#endif
	if (u.u_offset & BMASK || u.u_count & BMASK) {
		u.u_error = EINVAL;
		return;
	}
	if (physck (m320_sizes[UNIT(dev)][SLICE(dev)].nblocks, B_WRITE))
		physio (m320breakup, 0, dev, B_WRITE);
}


/********************************************************************
  io-control routine
*/

m320ioctl (dev, cmd, arg)
register 	dev, cmd;
register char	*arg;
{
	/* parms structure */
	union {
		int	pt_blk;		/* format track passes block number */
		struct {		/* format vol passes ..	*/
			int	pv_head;  /* ... heads */
			int	pv_cyl;   /* ... and cylinders */
		} parmv;
	} parm;

	register struct iobuf *cp, *up;
	register struct buf *bp;
	struct m3e_eca *ecp;
	int xbad;

#ifdef DEBUG
	if(M320Debug & DBIOCTL) {
		m320print(dev,"M320ioctl");
		printf("M320ctl cmd=%x arg=%s\n",cmd,arg);
	}
#endif
	/* general init	*/
	cp = &m320tab[CTL(dev)];
	up = &m320utab[CTL(dev)][UNIT(dev)];

	/* by command ... */

	if (cmd	 == M320FMTT || cmd == M320FMTV) {
		/* get parms from caller */
		if (copyin (arg, &parm, sizeof(parm))) {
			u.u_error = EFAULT;
			return;
		}

		/* get a buffer	*/
		bp = geteblk ();
		bp->b_flags |= B_PHYS | B_SPEC;
		bp->b_error = 0;
		bp->b_proc = u.u_procp;	 /* ? */
		bp->b_dev = dev;
		bp->b_bcount = 0;

		/* set flags for different commands */
		if (cmd == M320FMTT) {
			bp->b_flags |= B_FMTT;
		} else {

			bp->b_flags |= B_FMTV;  
		}
		if (cmd == M320FMTT) {
			/* setup block */
			bp->b_blkno = parm.pt_blk;
		}


		/* only	HDs take variable data */
		else if (!FLOPPY(dev)) {
			ecp = (struct m3e_eca *)up->Eca;
			up->Tpc = ecp->m3e_nhed = parm.parmv.pv_head;
			up->Cpv = parm.parmv.pv_cyl;
			bp->b_flags |= B_ABS;
		}

		/* send	to strategy, will filter B_NOCTL and invalid config state */
		m320strategy (bp);
		iowait (bp);

		/* error check */
		if (bp->b_flags	 & B_ERROR) {
			u.u_error = bp->b_error;
			goto done;
		}

		/* need	config write ? */
		if (cmd	 != M320FMTV || FLOPPY(dev))
			goto done;

		/* setup buffer	*/
		brelse (bp);
		bp = geteblk ();
		bp->b_flags |= B_PHYS | B_SPEC | B_ABS | B_CONF;
		bp->b_error = 0;
		bp->b_proc = u.u_procp;
		bp->b_dev = dev;
		bp->b_bcount = 0;

		/* send	to strategy */
		m320strategy (bp);
		iowait (bp);

		/* error check */
		if (bp->b_flags	 & B_ERROR) {
			u.u_error = bp->b_error;
			goto done;
		}
done:
		brelse (bp);
		/* end format */

	} else if(cmd == M320CHGBT){
		int	x;

		bp = geteblk();
		if(getbadtable(bp, dev, &xbad) == 0){
			/* remove the old bad track table from pool
		   	before inserting the new one. */
			if(up->Badep)
				baddel((char *) up->Badep);

			/* INTERRUPT LEVEL */
			x = splx(SPLEV(m320_ilev[CTL(dev)]));  
			if((up->Badep = badinit(bp->b_un.b_addr,xbad))
			 == NULL){
				up->b_flags |= B_NOCNFG;
				u.u_error = EIO;
			}
			else
				u.u_error = 0;
			splx(x);
		}
		else
			u.u_error = EIO;

		brelse(bp);
	/* bad command */
	} else
		u.u_error = EINVAL;

}

/* returns zero if bad track table is successfully read from disk and 
   placed in the buffer supplied by caller, else returns -1.
   The count of bad track table entries are placed into the location
   pointed to by "xbad".
*/
getbadtable(nbp, dev, xbad)
register struct buf *nbp;
register dev;
register *xbad;
{
	register struct buf *bp;
	register struct volumeid *vp;


	/* get a buffer	for the	config data */
	bp = geteblk();
	bp->b_flags  |= B_PHYS | B_ABS | B_READ;
	bp->b_error = 0;
	bp->b_proc = u.u_procp;
	bp->b_dev = dev;
	bp->Block = 0;	/* reading absolute block zero */

	/* send	to strategy */
	m320strategy (bp);
	iowait (bp);

	vp = (struct volumeid *)(bp->b_un.b_addr);
	/* error check,	signature string */
	if (bp->b_flags & B_ERROR || !m3sig (vp->vid_mac)) {
		brelse(bp);
		return(-1);
	}

	/* get bad track count,	skip if	zero */
	if ((*xbad = vp->vid_btl) == 0){
		brelse(bp); 
		return(-1);
	}

	/* ensure that all bad track data fits in a buffer */
	if (HASHSIZE + *xbad * sizeof(struct badent ) > SBUFSIZE) {
		/* geteblk init */
		printf ("M320: drive %d, ctl %d, bad track data	exceeds	%d buffer\n", UNIT(dev), CTL(dev), SBUFSIZE);
		brelse(bp);
		return(-1);
	}

	brelse(bp);

	/* now read absolute block 1 */
	bp = nbp;
	bp->b_flags |= B_PHYS | B_ABS | B_READ;
	bp->b_error = 0;
	bp->b_proc = u.u_procp;
	bp->b_dev = dev;
	bp->Block = 1;

	/* send	to strategy */
	m320strategy (bp);
	iowait (bp);

	if (bp->b_flags	& B_ERROR)
		return(-1);
	else
		return(0);
}

/********************************************************************
  low-level start/end routines (with bookkeeping)
*/

LOCAL m3sio (cp)  /* start */
register struct iobuf *cp;
{
	register struct m3e_eca *ecp;
	register struct iobuf *up;
	register struct buf *bp;

	/* ptr init */
	bp = (struct buf *)cp->Request;
	up = UNITP;
	ecp = (struct m3e_eca *)up->Eca;

	/* set the bit in blkacty (global system variable) to */
	/* indicate that there is activity on the mvme320     */
	blkacty |= (1 << M320);

	/* accounting */
	cp->Busy = 1;		 /* set	busy, reset timeout */
	cp->io_start = lbolt;	 /* actual i/o time start */
	up->io_start = lbolt;	/* actual i/o time start for device */

	/* controller setup */
	ecp->m3e_mnst = M3S_BUSY;
	ecp->m3e_exst = 0;
	ecp->m3e_phas = 0;
	((struct m3csr *)cp->Ioaddr)->regD = up->Umask;
}


LOCAL m3eio (bp)  /* end */
register struct buf *bp;
{
	/* Clear the bit in global system variable blkacty to */
	/* indicate no activity for mvme320 */
	blkacty &= ~(1 << M320);

	bp->b_flags &= ~(B_SPEC | B_ABS | B_FINAL);  /*	local flags off	*/
	iodone (bp);				     /*	release	buffer	*/
}


/********************************************************************
  interrupt handler
*/

m320intr (devn)	 /* first-level, sets timer lockout */
register devn;  /* first dev number on interrupting controller; 0, 32, etc. */
{
#ifdef DEBUG
	if(M320Debug & DBINTR)
		m320print(devn,"M320intr");
#endif
	/* set timer lockout semaphore */
	timelock = 1;

	/* call	int handler proper */
	m3inth (devn/32);  /*	convert	to controller number */

	/* release lock	*/
	timelock = 0;
}


LOCAL m3inth (ctl)
register 	ctl;
{
	register struct m3e_eca *ecp;
	register struct buf *bp;
	register struct iobuf *cp, *up;
	register struct m3csr *iop;
	register struct iotime *ip;
	register 	imask;

       /*
	*  WARNING! The use of static recalcnt can theoretically,
	*   but not probably, cause a missed or unneeded
	*   recalibration.
	*/
	static int	recalcnt;


	/* see if there	*/
	iop = (struct m3csr *)(cp = &m320tab[ctl])->Ioaddr;
	if ((cp->b_flags & B_NOCTL) || (!cp->Busy)) {
		/* try to clear	int anyway */
		bprobe (&iop->regD, 0);

		/* ignore first unexpected interrupt(possible boot remnant) */
		if (! cp->Kluge)
			cp->Kluge = 1;
		else
			printf ("M320: ctl %d, spurious	interrupt\n", ctl);
		return;
	}

	imask = iop->regB & 0xf0;
	/* clear interrupt */
	iop->regD = 0;


	/* address unit	*/
	

	bp = (struct buf *)cp->Request;
	up = UNITP;                                                      

	/* validate unit */
	if (!(imask & up->Umask)) {
		printf ("M320: drive %d, ctl %d, mask 0x%x, interrupt not on active unit\n",
		     		    UNIT(bp->b_dev), ctl, imask);
		return;
	}
	if (imask != up->Umask) {
		printf ("M320: drive %d, ctl %d, mask 0x%x, extra interrupt\n",
		     		    UNIT(bp->b_dev), ctl, imask);
		/* note: continue */
	}

	/* set up to accumulate actual i/o time */
	ip = (struct iotime *)up->Statp;

	ecp = (struct m3e_eca *)(UNITP)->Eca;

#ifdef DEBUGERR
/* fake a soft error on every 100th read or write command  */
	if ( ( ecp->m3e_cmd == M3C_REMS) || (ecp->m3e_cmd == M3C_WRMS) )  {
		cp->io_nreg++;
		if ( cp->io_nreg%100 == 0 ) {
			m320elog(ecp,up,IO_BOFF);
		}
	}
#endif


	/* ********************************************** */
	/* if error, check for retries and/or recalibrate */
	/* ********************************************** */


	if ( ( ecp->m3e_mnst )
	|| ( ecp->m3e_exst )
	|| ( ( (ecp->m3e_cmd == M3C_REMS) || (ecp->m3e_cmd == M3C_WRMS) )
			&& ( ecp->m3e_bufl != ecp->m3e_actr) )
	|| ( (ecp->m3e_cmd == M3C_REMS) && ( (ecp->m3e_eccr[0] != 0)
		|| (ecp->m3e_eccr[1] != 0) || (ecp->m3e_eccr[2] != 0) ) )) {

	/* **************************************************** */
	/* Here because there was an error, either hard or soft */
	/* **************************************************** */

		/* log the error */
		m320elog(ecp,up,IO_BOFF);

		/* if error during a recalibrate, and retries left, retry */
		if ( ecp->m3e_cmd == M3C_CALB ) {
			if ( --recalcnt	) {
				m3sio(cp);
				return;
			}
		}

		/* else if position error - recalibrate */
		else if ( ecp->m3e_exst & POSERR && ( --cp->b_errcnt > 0)  ) {
			recalcnt = RECALCNT;
			ecp->m3e_cmd = M3C_CALB;
			m3sio(cp);
			return;
		}

		/* else not a recalibrate, and not a position error, but */
		/* if retry can be done, do it */
		else if ( ( --cp->b_errcnt > 0) && (ecp->m3e_mnst & RETMSK) ) {

			bp->b_flags |= B_ERROR;
			m3start(ctl);
			return;
	 	}  


		/* ********************************************** */
		/* 	Here for hard errors !!!                  */
		/* ********************************************** */
		/* recalibrate  has been done 3 times - real error */
		/* or */
		/* retry count exhausted */                                
		/* or */
		/* this is an error for which there is no point in retrying */


		bp->b_flags |= B_ERROR;
		bp->b_error = EIO;

		/* say nothing done */
		bp->b_resid = bp->b_bcount; 

		printf ("M320: drive %d, ctl %d, hd %d,	cyl %d,	cmd %d,	main %d, ext 0x%x, i/o error\n",
	        UNIT(bp->b_dev), ctl, ecp->m3e_head,
	        ecp->m3e_cyl, ecp->m3e_cmd, ecp->m3e_mnst,
	        ecp->m3e_exst);
		/* (0 is cyl number not used here) */

	} else if (ecp->m3e_cmd == M3C_CALB) {
	/* ***************************************************************** */
 	/* Here because there are no errors on a recalibrate, must retry now */
	/* ***************************************************************** */

		bp->b_flags |= B_ERROR;     /* set error flag for m3start */
		m3start(ctl);
		return;

	} else if ( !(bp->b_flags & B_FINAL))  {
        /* **************************************************************** */
	/* Here because no errors, not the last io of request, must do more */
        /* **************************************************************** */

		m3start(ctl);
		return;
	} else { 
	/* ****************************************************** */
        /*  Here because there are no errors, last i/o of request */
	/* ****************************************************** */

		bp->b_resid = 0;

	}

/* ********************************************************************* */
/* Here because the io request has completed, either successfully or not */
/* ********************************************************************* */


	/* turn off Busy, go on to next request in queue */
	cp->Busy = 0; 

	/* close error log on this request */
	logberr	(up,bp->b_flags & B_ERROR);

	/* dequeue the buffer */
	up->b_actf = bp->Qnext;
	up->Qcnt--;
	if (bp == (struct buf *)up->Acts)
		up->Acts = (int)up->b_actf;

			
	/* log read/write I/O times */
	if( !( bp->b_flags & NON_RWIO) ) {

		/* total request time */
		ip->io_resp += lbolt - bp->b_start; 
		/* actual I/O time for last command that went through m3sio */
		ip->io_act += lbolt - up->io_start;

	}

	m3eio (bp);  /*	end i/o	*/
          

	/* start another i/o or restart this one depending on Busy */
  	m3start (ctl);


}


/********************************************************************
  dump routines
*/

m320dump ()  /*	driver dump vectored-to	by OS */
{
	extern int	m3doio ();
	register struct iobuf *cp;
	register struct m3csr *iop;
	register ctl, x;

	/* kill	interrupts */
	spl7 ();  /* INTERRUPT LEVEL */

	/* stop	all controllers	*/
	for (ctl = 0, cp = m320tab; ctl < actctl; ++cp, ++ctl)
		bprobe (&((struct m3csr *)cp->Ioaddr)->regD, 0);

	/* wait	for things to settle */
	for (x = 1000000; x; --x);

	/* clear interrupts */
	for (ctl = 0, cp = m320tab; ctl < actctl; ++cp, ++ctl)
		bprobe (&((struct m3csr *)cp->Ioaddr)->regD, 0);


	/* invoke general dump routine in kernel */
	/* memdump	(m3doio); */

}


LOCAL m3doio (dev, blkno, maddr, rdwr)	/* write(/read?) a block for memdump */
register dev;
register 	blkno;
register char	*maddr;
register 	rdwr;
{
	static struct iobuf *cp, *up;
	static struct m3csr *iop;
	static struct m3e_eca *ecp;
	static struct size *sp;
	static int	xsect, xalt, xtrack, xcyl, rr;

	static int	state = 0;

	switch (state) {
	case 0:	/* initialize, drop into first i/o */
		 {
			cp = &m320tab[CTL(dev)];
			up = &m320utab[CTL(dev)][UNIT(dev)];
			sp = &m320_sizes[UNIT(dev)][SLICE(dev)];
			if (!FLOPPY(dev) && up->b_flags	 & B_OPEN)
				/* okay	*/;
			else
			 {
				printf ("M320: dump device 0x%x04 not available\n",
				     dev);
				state = 	2;
				return;
			}
			iop = (struct m3csr *)cp->Ioaddr;
			ecp = (struct m3e_eca *)up->Eca;
			state = 	1;
		}
	case 1:	/* do a	block i/o */
		 {
			/* prevent slice overflow */
			if (blkno >= sp->nblocks) {
				printf ("M320: dump truncated at end of	logical	device\n");
				printf ("      %d blocks written\n", sp->nblocks);
				state = 	2;
				return;
			}
			blkno = 	(blkno + sp->blkoff) * HSPU;

			/* retry loop */
			rr = 5;
			do
			 {
				/* setup eca */
				ecp->m3e_cmd = (rdwr) ? M3C_REMS : M3C_WRMS;
				ecp->m3e_mnst = M3S_BUSY;
				ecp->m3e_exst = 0;
				ecp->m3e_bufa = (uint32)maddr;
				ecp->m3e_bufl = BSIZE;
				xtrack = blkno / HSPT;
				xsect = blkno - xtrack * HSPT;
				if (badchk ((char *)up->Badep, xtrack, 1,
				     &xalt) == 0)
					xtrack = xalt;
				ecp->m3e_sect = xsect + 1;
				xcyl = xtrack / up->Tpc;
				ecp->m3e_head = xtrack - xcyl * up->Tpc;
				ecp->m3e_cyl = xcyl;
				ecp->m3e_phas = 0;

				/* start i/o, wait for completion */
				iop->regD = up->Umask;
				while (ecp->m3e_mnst == M3S_BUSY)
					;
				iop->regD = 0;	/* clear interrupt ? */
			}      while (ecp->m3e_mnst && --rr);

			if (ecp->m3e_mnst)  /* error? */ {
				printf ("M320: dump i/o	error, terminating\n");
				state = 2;
			}
			return;
		}
	case 2:	/* error, no-op	*/
		return;
	}  /* end switch */
}


/********************************************************************
  timer	routines
*/

LOCAL m3timer ()  /* timeout interval routine */
{
	register struct iobuf *cp;
	register struct buf *bp;
	register int	ctl;
	register 	xlev;
	register struct iobuf *up;
	int d;

	/* schedule next hit */
	timeout	(m3timer, 0, 100 * HZ);

	/* disable for low-priority mode */
	xlev = spl6 ();	 /* INTERRUPT LEVEL */

	/* if locked out, skip this time */
	if (timelock) {
		splx (xlev);  /* INTERRUPT LEVEL */
		return;
	}

	/* poll	controllers */
	for (ctl = 0, cp = m320tab; ctl < actctl; ++cp, ++ctl) {
		/* if device busy then increment and test count	*/
		if (cp->Busy && ++cp->Busy > 2)	{
#ifdef DEBUGQ
	printf("Timeout occurred\n");
#endif
			/* one	full timeout period */

			/* we have not yet dequeued the request so */
			/* it is not a special case */
			/* delete all pending requests */
			/*
			/* bp = (struct buf *)cp->Request; */
			/* bp->Qnext = NULL; */
			/* m3flush	(bp); */

		 	for (d = 0; d < MAXUNIT; d++) {

 				up = &m320utab[ctl][d];
				logberr(up,B_ERROR);
				bp = up->b_actf;
					

				m3flush(bp);
				up->Qcnt = 0;

			}
			cp->Busy = 0;

			/* kill	controller, failed unit	would have given status	*/
			cp->b_flags |= B_NOCTL;
			printf ("M320: controller %d timed-out,	disabled\n",
			     ctl);
		}
        }

	/* restore priority */
	splx (xlev);  /* INTERRUPT LEVEL */

}


LOCAL m3flush (bp)  /* flush a queue of	request	bufs */
register struct buf *bp;
{
	register struct buf *xp;

	while (bp != NULL) {
		/* see normal termination in m3inth */
		bp->b_flags |= B_ERROR;
		bp->b_error = EIO;
		if (bp->b_flags	 & B_SPEC)
			bp->b_resid = 0;
		else
			bp->b_resid = bp->b_bcount;
		xp = (struct buf *)bp->Qnext;
		m3eio (bp);
		bp = xp;
	}
}

/*
 *	m320elog: performs error logging
 */

#ifdef  DEBUG 
	int  m320ecnt;		/* error counter */
#endif

m320elog(ecp,up,blkoff)
register struct m3e_eca *ecp;
register struct iobuf *up;
int blkoff;
{
	register struct m320eregs *erp;

	erp = (struct m320eregs *)up->io_addr;

	erp->er_cmd = ecp->m3e_cmd; 	/* command to controller */
	erp->er_mnst = ecp->m3e_mnst;	/* main status */
	erp->er_exst = ecp->m3e_exst;	/* extended status */
	erp->er_bufl = ecp->m3e_bufl;	/* requested buffer length */
	erp->er_actr = ecp->m3e_actr;	/* actual number of bytes transfered */
	erp->er_eccr[0] = ecp->m3e_eccr[0];  /* ecc remainder */
	erp->er_eccr[1] = ecp->m3e_eccr[1];  /* ecc remainder */
	erp->er_eccr[2] = ecp->m3e_eccr[2];  /* ecc remainder */

#ifdef  DEBUG
	m320ecnt++;			/* error counter */
#endif
	/* get major number of block device for error logging */
	up->b_dev = makedev( M320, minor(up->b_dev) );

	fmtberr(up,blkoff);			/* post to the error log */
}
