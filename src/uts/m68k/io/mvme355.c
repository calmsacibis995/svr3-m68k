/***
 ***      mvme355.c    2.0     86/03/11       
 ***
 ***
 ***
 *** Nine-track tape driver for the Motorola mvme355 controller.
 ***
 ***        *=========================================*
 ***        *	Conforms to System 5 standards        *
 ***	    *   runs with rev 020 firmware (or later) *
 ***        *=========================================*
 ***
 *** Device decriptors:
 ***    Major device tags the driver for system reference.
 ***    Minor device definitions:  (Positive true logic)
 ***      BITS      7    -   Controller bit 1    -   Max two controlers
 ***
 ***                6    -   Drive bit 2     
 ***                5    -   Drive bit 1    
 ***                4    -   Drive bit 0         -   Max eight drives
 ***
 ***                3    -   High speed select
 ***                2    -   High density select
 ***                1    -   Swap bytes select
 ***                0    -   Rewind on close select
 ***
 ***
 *** BUGS:
 ***     Of course.
 ***     Multi controlers are not implemted yet.
 ***/

#include <sys/param.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/fs/s5dir.h>
#include <sys/signal.h>
#include <sys/user.h>
#include <sys/errno.h>
#include <sys/buf.h>
#include <sys/elog.h>
#include <sys/iobuf.h>
#include <sys/systm.h>
#include <sys/dk.h>

#include "sys/mvme355.h"
/* if PHYSBSIZE changes then also change in physdsk.c */
#define PHYSBSIZE	(64 * 1024)		/* size of physical buffer */
#define OK         	0			/* ok status */
#define NOK        	1			/* bad status */
#define TRUE       	1                                 
#define FALSE      	0
#define DIAG		if(debug) printf	/* diagnostic printf */
#define DEBUG		if(debug)		/* diagnostic function */
#define NUNIT		8			/* max units/controller */
#define NTP		12			/* max units/controller */
#define TPPRI		PZERO-1			/* sleep priority */
#define FWRITE		2		/* open for writeing */
#define FREAD		3			/* open for reading */
/*define MT_XXBIT	MT_16BIT		/* m68010 system 16 bit mem*/
#define MT_XXBIT	MT_32BIT		/* m68020 system 32 bit mem*/

/*
 * Meaningful device types in the minor device
 */
#define TPUNIT(dev)  ( (minor(dev) & 0x70) >>3)  /* minor device number */

#define HISPD_CHK(dev)    (minor(dev) & 8)    /* Hi-speed         */
#define HIDEN_CHK(dev)    (minor(dev) & 4)    /* Hi-density       */
#define SWAPB_CHK(dev)    (minor(dev) & 2)    /* Byte swap        */
#define REWND_CHK(dev)    (minor(dev) & 1)    /* Rewind-on-close  */
/*
 * States for the b_active flag
 */
#define INACTIVE       0                         /* request inactive state */
#define ACTIVE         1                         /* request active state */
#define QUEUED         2                         /* active-queued state  */

static char debug = FALSE;

/*
 * TP/tp controler/device driver structure - system interface.
 */
extern char *m355_addr;
extern int  m355_bmj;		/* Block Major Device Number */
extern int  m355_cnt;		/* Count */
extern int  m355_ilev[];		/* Interrupt Levels */
extern int  m355_ivec[];		/* Interrupt Vectors */

/*
 * Definition of a unit
 */
static struct  unit {
        char    un_present;     /* physical unit is present */
        char    un_opened;      /* lock against multiple opens */
        char    un_busy; 	/* sleep/wakeup flag */
        char    un_rewind;	/* unit rewinding ?? */
        char    un_crfm;	/* unit has crossed a file mark on a read */
        long    un_flags;	/* flags like bp     */
}tpunit[8];


/*
 * Data per controller
 */
static struct  ctlr
{
        struct unit     *c_units[NUNIT];/* units on controller */
        struct vtape_shio       *c_io;  /* ptr to I/O space data */
        caddr_t         c_iopb;         /* ptr to IOPB */
        caddr_t         c_uib;          /* ptr to UIB */
        short           c_type;         /* controller type */
	char		c_que;		/* somthing's has the for controller */
	char		c_qued;		/* somthing's waiting for controller */
        char            c_present;      /* controller is present */
        u_char          c_vec;          /* interrupt vector */
        u_char          c_lev;          /* interrupt level */
        
	/* current transfer: */
        struct buf *    c_bp;           /* physical buffer address */
        daddr_t         c_count;        /* current count */
        short           c_cmd;          /* current command */
        char            c_unit;         /* current unit */
        short           c_retries;      /* retry count */
        char            c_wantint;      /* expecting interrupt */
        u_long		c_dmaddr;       /* dma address */
        short           c_recdcnt;      /* record count */
        short           c_fmkcnt;       /* filemark count */
        int             c_byterecd;     /* bytes per record */
        char            c_memtype;      /* memory type */
        char            c_addrmod;      /* address modifier */
        char            c_recovery;     /* error recovery */
} tpctlrs;

/*
 *    misc globals (to many!!)
 */ 
static int mtflag;

/*
 * Default UIB
 */
static struct uib default_uib = {
        0,                              /* J1-14 select                      */
        0,                              /* J1-16 select                      */
        0,                              /* J1-36 select                      */
        0,                              /* J1-44 select                      */
        0,                              /* J2-26 select                      */
        0,                              /* J2-50 select                      */
        0,                              /* reserved (must be 0)              */
        0,                              /* reserved (must be 0)              */
        0,                              /* reserved (must be 0)              */
        0,                              /* reserved (must be 0)              */
        0,                              /* reserved (must be 0)              */
        3,                              /* retry count                       */
        0,                              /* reserved (must be 0)              */
        0,                              /* reserved (must be 0)              */
        ATT_ERSEN | ATT_RTRY | ATT_STC,
		                        /* attribute flags status chng       */
        0xff,                           /* 100 % of buffer fill              */
        0,                              /* stat chng rewind complete level   */
        0,                              /* stat chng rewind complete vector  */
        0,                              /* stat chng level                   */
        0                               /* stat chng vector                  */
};


/*
 * Known error types
 */
#define N_KN_ERR  26

static char errknown[N_KN_ERR] =
{ 0x10,0x14,0x15,0x17,0x18,0x1A,0x20,0x21,0x23,0x25,0x42,0x52,0x58,0x5A,
0x5B,0x5C,0x5D,0x60,0x61,0x62,0x63,0x64,0x65,0x70,0x80,0xFF };

/*
 * English translation of known error codes
 */
static char *  err[] = {
"tape not ready",                      /* 0x10 */
"invalid command code",                /* 0x14 */
"illegal fetch & execute command",     /* 0x15 */
"illegal memory type",                 /* 0x17 */
"bus timeout",                         /* 0x18 */
"tape write protected",                /* 0x1A */
"end of tape",                         /* 0x20 */
"load point error",                    /* 0x21 */
"uncorrectable data error",            /* 0x23 */
"data block on tape is longer than requested",            /* 0x25 */
"signal definition error",             /* 0x42 */
"VME bus error",                       /* 0x52 */
"FIFO error",                          /* 0x58 */
"invalid address modifier",            /* 0x5A */
"invalid memory address",              /* 0x5B */
"invalid interrupt level",             /* 0x5C */
"illegal DMA burst count",             /* 0x5D */
"IOPB failed",                         /* 0x60 */
"DMA failed",                          /* 0x61 */
"invalid UIB parameter",               /* 0x62 */
"tape timeout",                        /* 0x63 */
"invalid IOPB parameter",              /* 0x64 */
"invalid record size",                 /* 0x65 */
"time out on rewind",                  /* 0x70 */
"command aborted",                     /* 0x80 */
"command not implemented",             /* 0xFF */
"??? unknown error"                    /* anything else */
};

static long     vtp_uib;              /* short io address of uib          */
static char     null_data = FALSE;      /* no-data only                     */

static struct buf rtpbuf;               /* buf for no-data */  
static struct buf *static_bp = &rtpbuf; /* buf for no-data */  

/*
 * Macros for extracting vtape short I/O locations
 */
#define CSR(x)  (c->c_io->vtp_cmd[x].cstatus)
#define IOPBUNIT(x) (c->c_io->vtp_cmd[x].pb.pb_coptn & 0x7)
#define IOPB(x) (c->c_io->vtp_cmd[x].pb)
#define TSR(x)  (c->c_io->vtp_sreg[x])
#define SCR     (c->c_io->vtp_screg)
#define HEAD    (c->c_io->vtp_head)
#define TAIL    (c->c_io->vtp_tail)

/*
 * m355init()  ---Set up initialize parameters here.
 *           
 *            
 */
m355init()
{
register struct uib  *ub;
register struct uib  *dfub;
register struct ctlr *c = &tpctlrs;
register struct iopb *ip;
register int unit,counter;

	/*
 	 * Save off the address of the device
 	 */
	(u_short *)c->c_io = (u_short *)m355_addr;

	/*
	 * Test the water to see if the board is in the pond
	 */
	if ( bprobe(m355_addr,-1)) {
		c->c_present = FALSE;
		return;
	}

	DIAG("TPINIT: \t CSR(0)=%x &(CSR(0))=%x \n",CSR(0), &(CSR(0)));

	/*
	 *  reset the controller
 	*/
	CSR(0) = CS_BDCLR;
	tpdelay(10000);
	CSR(0) ^= CS_BDCLR;
	tpdelay(10000);
	
	counter=0;
	while( (CSR(0) != (CS_BOK|CS_DONE) ) && ++counter>0) {
		tpdelay(30);
	}	
	if ( (counter<0) || CSR(0) != (CS_BOK|CS_DONE) ) {
		DIAG("\nm355: board failed power up diagnostics\n");
		return(0);
	}

	CSR(0) ^= CS_DONE;		/* clear board for next command */
	CSR(0) |= CS_SLED;              /* set LED to green             */

	/*
	 * Store address to iopb and uib
	 */
	c->c_iopb = (caddr_t)&(c->c_io->vtp_cmd[0].pb);
	c->c_uib  = (caddr_t)&(c->c_io->vtp_xfer[0]);

	c->c_present = TRUE;
	c->c_que     = FALSE;
	c->c_qued    = FALSE;

	/* 
	* set up for vectored interrupts
	*/
	c->c_vec = (u_char )(m355_ivec[0] / 4);/* only interrupt vector */
	c->c_lev = (u_char )m355_ilev[0];    /* one and only interrupt level */
	
	dfub = &default_uib;
	dfub->uib_lv_rw = c->c_lev;		/* rewind quick status change */
	dfub->uib_vc_rw = c->c_vec;		/* vector and level	      */

	dfub->uib_lev = c->c_lev;		/* status change              */
	dfub->uib_vec = c->c_vec;		/* vector and level	      */
	
	DIAG(" c_vec=%x, c_lev=%x",c->c_vec,c->c_lev);
	
	/*
	* init static fields in iopb
	*/
	ip = (struct iopb *)c->c_iopb;
	ip->pb_n_vec = c->c_vec;
	ip->pb_e_vec = c->c_vec;
	/*
	* initialize unit
	*/
	u_st_init(0,TRUE);
	u_st_init(1,TRUE);
	u_st_init(2,TRUE);
	u_st_init(3,TRUE);
	u_st_init(4,TRUE);
	u_st_init(5,TRUE);
	u_st_init(6,TRUE);
	u_st_init(7,TRUE);

	c->c_unit = 0;
	unitinit(c);
	if (tpcmdst(c)) {
		DIAG("==>can't initialize tape drive 0\n");
		u_st_init(0,FALSE);
		return; 
	}
	c->c_unit = 1;
	unitinit(c);
	if (tpcmdst(c)) {
		DIAG("==>can't initialize tape drive 1\n");
		u_st_init(1,FALSE);
		return; 
	}
	c->c_unit = 2;
	unitinit(c);
	if (tpcmdst(c)) {
		DIAG("==>can't initialize tape drive 0\n");
		u_st_init(0,FALSE);
		return; 
	}
	c->c_unit = 3;
	unitinit(c);
	if (tpcmdst(c)) {
		DIAG("==>can't initialize tape drive 0\n");
		u_st_init(0,FALSE);
		return; 
	}
	c->c_unit = 4;
	unitinit(c);
	if (tpcmdst(c)) {
		DIAG("==>can't initialize tape drive 0\n");
		u_st_init(0,FALSE);
		return; 
	}
	c->c_unit = 5;
	unitinit(c);
	if (tpcmdst(c)) {
		DIAG("==>can't initialize tape drive 0\n");
		u_st_init(0,FALSE);
		return; 
	}
	c->c_unit = 6;
	unitinit(c);
	if (tpcmdst(c)) {
		DIAG("==>can't initialize tape drive 0\n");
		u_st_init(0,FALSE);
		return; 
	}
	c->c_unit = 7;
	unitinit(c);
	if (tpcmdst(c)) {
		DIAG("==>can't initialize tape drive 0\n");
		u_st_init(0,FALSE);
		return; 
	}

	DIAG("---exiting from m355init without error\n");
}

/**  **/
/**
 ** m355open()  ---KERNEL DRIVER ENTRY POINT
 **
 ** FUNCTION: Accessed whenever device file is opened
 **           The tape is assumed to be positioned as desired.
 **           Sets drive initialization enviroment for any "special"
 **           special devices.
 **
 ** ENTRY:    dev   -   Device to open
 **           flag  -   What mode to open in
 **
 ** EXIT:     0     -   Successful open
 **           otherwise unsuccessful open with error status
 **
 **/
m355open(dev, flag)
register dev_t dev;
register int flag;
{
register struct unit *un;
register struct ctlr *c = &tpctlrs;
register int i, retry, unit;

  	DIAG("TPOPEN: entered, dev=%x, flag=%x\n",dev,flag);

	if ( !c->c_present ) {
  		DIAG("TPOPEN: , board not here");
		u.u_error = EIO;
    		return;			/* board not on the buss */
	}
	unit = TPUNIT(dev);
	un = &tpunit[unit];

  	DIAG("TPOPEN: entered, TSR(unit)=%x\n",TSR(unit));
	if( !((TSR(unit) & SR_ONLN) && (TSR(unit) & SR_RDY)) ) {
		u.u_error = EACCES;
    		return;			/* tape drive is off line */
	}
	if (un->un_opened) {
		u.u_error = EACCES;
		return;				/* already opened */
	} else {
		un->un_opened = TRUE;		/* set opened flag */
		un->un_crfm   = FALSE;		/* set crossed file mark flag */
		u.u_error = 0;
	}
	un->un_rewind = FALSE;

	tprque(c); 
	c->c_wantint = TRUE;
	null_data = TRUE;
	/*
	 * Check for a "special" special device
	 */
	unitinit(c);
	if (SWAPB_CHK(dev)) 
		tpswab(c);
	if (HIDEN_CHK(dev)) 
		tphiden(c);
	if (HISPD_CHK(dev)) 
		tphispd(c);
	tpcmdslp(c);

	/* c->c_que = FALSE; */

	DIAG("---exiting from m355open without error\n");
	return(OK);
}

/**  **/
/**
 ** m355close()  ---KERNEL DRIVER ENTRY POINT
 **
 ** FUNCTION:  We done.
 **            When a file is closed, a double end-of-file is written if
 **            the file was opened for writing.  If the file was normal-
 **            rewind, the tape is rewound.  If it is no-rewind and the 
 **            file was open for writing, the tape is positioned before
 **            the second EOF just written.  If the file was no-rewind
 **            and opened read-only, the tape is positioned after the EOF
 **            following the data just read.  Once opened, reading is
 **            restricted to between the position when opened and the
 **            next EOF of the last write.  The EOF is returned as a zero-
 **            length read.
 **            Reset the open flag for the device.
 **
 ** ENTRY:     dev   -   Device to close
 **            flag  -
 **
 ** EXIT:
 **
 **/
m355close(dev)
register dev_t dev;
{
register struct unit *un;
register struct ctlr *c = &tpctlrs;
register int unit;

	DIAG("TPCLOSE: minor(dev)=%x\n", minor(dev));

	unit = TPUNIT(dev);
	un = &tpunit[unit];

	if ( (un->un_opened == FWRITE) && !(mtflag) ) {
		tprque(c); 
		c->c_unit = unit;
		c->c_cmd  = C_WRTFMK;
		c->c_fmkcnt = 2;
		null_data = TRUE;
		if (tpcmdslp(c)) DIAG("\nm355: close error on write eof\n");
      
		if ( REWND_CHK(dev) ) {
			tprque(c); 

			c->c_unit = unit;
			c->c_cmd  = C_REWIND;
			null_data = TRUE;
			if (tpcmdslp(c)) 
				DIAG("\nm355: close error on rewind\n");
		} else {
			tprque(c); 
			c->c_unit = unit;
			c->c_cmd  = C_MBWDFM;
			c->c_fmkcnt = 1;
			null_data = TRUE;
			if (tpcmdslp(c)) DIAG("\nm355: close error on bsf\n");
		}
	} else
		if ( (un->un_opened == FREAD) && !(mtflag) ) {
			if ( REWND_CHK(dev) ) {
			 	tprque(c); 
				c->c_unit = unit;
				c->c_cmd  = C_REWQCK;
				null_data = TRUE;
				if (tpcmdslp(c))
					DIAG("\nm355: close error on rewind\n");
			} else {
				if( !un->un_crfm ){
			 		tprque(c); 
					c->c_unit = unit;
					c->c_cmd  = C_MFWDFM;
					c->c_fmkcnt = 1;
					null_data = TRUE;
					if (tpcmdslp(c))
						DIAG(
						"\nm355: close error on fsf\n");
				}
			}
		} else
			if ( REWND_CHK(dev) ) {
				tprque(c); 
				c->c_unit = unit;
				c->c_cmd  = C_REWQCK;
				null_data = TRUE;
				if (tpcmdslp(c))
				DIAG("\nm355: close error on rewind\n");
			}
		if ( mtflag ) mtflag = FALSE;
		un->un_opened = FALSE;
		DIAG("---exiting m355close\n");
}

/**
 ** m355ioctl()  ---KERNEL DRIVER ENTRY POINT
 **
 ** FUNCTION:  Process "mt" and other requests from system.
 **
 ** ENTRY:     dev  -  Device opened for operation.
 **            cmd  -  What to do, case for the outer switch below.
 **            addr -
 **            flag -
 **
 ** EXIT:
 **
 **/
m355ioctl(dev, cmd, addr, flag)
register dev_t dev;
register u_long addr;
register int cmd,flag;
{
register struct ctlr *c;
register struct unit *un;
register char buff[32];
register struct iopb *ip;
register struct uib  *ub, *dfub;

	DIAG("TPIOCTL: entered, dev=%x, cmd=%x, addr=%x, flag=%x\n"
		,dev,cmd,addr,flag);

	bzero(buff,32);
	c = &tpctlrs;
	mtflag = TRUE;
	null_data = TRUE;  
	un = &tpunit[TPUNIT(dev)];
	tprque(c);
	c->c_unit     = TPUNIT(dev);
	c->c_memtype  = MT_XXBIT;
	c->c_addrmod  = AM_SD_ND;
  
	DIAG("  mag tape I/O control operation =%x\n ",cmd);

	switch (cmd) {
		case MTREW:
			DIAG("MTREW\n");
			c->c_cmd = C_REWQCK;
			if (tpcmdslp(c)) u.u_error = EIO;
			break;
		case MTWEOF:
			DIAG("MTWEOF\n");
			c->c_cmd = C_WRTFMK;
			c->c_fmkcnt = 1;
			if (tpcmdslp(c)) u.u_error = EIO;
			break;
		case MTFSF:
			DIAG("MTFSF\n");
			c->c_cmd = C_MFWDFM;
			c->c_fmkcnt = 1;
			if (tpcmdslp(c)) u.u_error = EIO;
			break;
		case MTBSF:
			DIAG("MTBSF\n");
			c->c_cmd = C_MBWDFM;
			c->c_fmkcnt = 1;
			if (tpcmdslp(c)) u.u_error = EIO;
			break;
		case MTFSR:
			DIAG("MTFSR\n");
			c->c_cmd = C_MFWDBK;
			c->c_recdcnt = 1;
			if (tpcmdslp(c)) u.u_error = EIO;
			break;
		case MTBSR:
			DIAG("MTBSR\n");
			c->c_cmd = C_MBWDBK;
			c->c_recdcnt = 1;
			if (tpcmdslp(c)) u.u_error = EIO;
			break;
		case MTOFFL:
			DIAG("MTOFFL\n");
			c->c_cmd = C_OFFLN;
			if (tpcmdslp(c)) u.u_error = EIO;
			break;
		case MTNOP:
			DIAG("MTNOP\n");
			c->c_cmd = C_DIAG;
			if (tpcmdslp(c)) u.u_error = EIO;
			break;
		case MTERASE:
			DIAG("MTERASE\n");
			c->c_cmd = C_ERSEOT;
			if (tpcmdslp(c)) u.u_error = EIO;
			break;
/*		case MTIOCGET:
			mtget = (struct mtget *)addr;
			DIAG("    mag tape status command\n");
			mtget->mt_type   = MT_ISTP;           /* from mtio.h */
/*			mtget->mt_dsreg  = c->c_io->vtape_cmd[0].pb.iopb_status;
			mtget->mt_erreg  =
				 c->c_io->vtape_cmd[0].pb.iopb_error_code;
			mtget->mt_resid  = 0;
			mtget->mt_fileno =
				 c->c_io->vtape_cmd[0].pb.iopb_fmk_count;
			mtget->mt_blkno  =
				 c->c_io->vtape_cmd[0].pb.iopb_recd_count;
			break;
*/
		case IOMESS:
			debug ^= TRUE;
			DIAG("\nm355ioctl: message toggle, debug = %x\n",debug);
			c->c_que = FALSE;
			break;
/*		case IOIOPB:
			DIAG("\nm355ioctl: iopb fondling\n");
			ip = (struct iopb *)buff;
			copyin(addr, ip, sizeof(struct iopb));
			c->c_cmd       = ip->iopb_cmd;
			c->c_recdcnt   = ip->iopb_recd_count;
			c->c_fmkcnt    = ip->iopb_fmk_count;
			c->c_memtype   = ip->iopb_mem_type;
			c->c_addrmod   = ip->iopb_addr_mode;
			burst          = ip->iopb_dma_burst;
			level          = ip->iopb_int_level;
			c->c_wantint   = TRUE;
			if (tpcmdslp(c)) u.u_error = EIO;
			break;
		case IOUIB:
			DIAG("\nm355ioctl: changing uib\n");
			dfub = &default_uib;
			ub = (struct uib *)buff;
			copyin(addr, ub, sizeof(struct uib));
			dfub->uib_J1_14         =    ub->uib_J1_14;
			dfub->uib_J1_16         =    ub->uib_J1_16;
			dfub->uib_J1_36         =    ub->uib_J1_36;
			dfub->uib_J1_44         =    ub->uib_J1_44;
			dfub->uib_J2_26         =    ub->uib_J2_26;
			dfub->uib_J2_50         =    ub->uib_J2_50;
			dfub->uib_retry         =    ub->uib_retry;
			dfub->uib_attrib        =    ub->uib_attrib;
			break;
		case IOUNLOCK:
			DIAG("\nm355ioctl: unlock open\n");
			un->un_opened = FALSE;
			break;
		case IOSTAT:
			DIAG("\nm355ioctl: tape status\n");
			sreg_image = (short)TSR(c->c_unit);
			copyout(sreg_image, addr, 2);
			break;
		case IOINIT:
			DIAG("\nm355ioctl: tape init\n");
			c->c_wantint   = TRUE;
		        m355init(c);
		        if (tpcmdslp(c)) u.u_error = EIO;
			break;
			case IORESET:
			DIAG("\ntpioctl: tape reset\n");
			CSR(0) |= CS_BDCLR;
			tpdelay(1000);
			CSR(0) &= ~CS_BDCLR;
			break;
*/
		default:
			DIAG("m355ioctl: unrecognized command type\n");
			c->c_que = FALSE;
			u.u_error = EINVAL;
	}
	if (u.u_error) DIAG("---exiting m355ioctl with u.u_error=%x\n",u.u_error);
	else           DIAG("---exiting m355ioctl without error\n");
}

/**
 ** m355strategy()  ---KERNEL DRIVER ENTRY POINT (from physio)
 **
 ** FUNCTION:     Do range checking on I/O parameters and
 **               start unit or controller command.
 **
 ** ENTRY:        bp  -  Pointer to I/O packet from system.
 **
 ** EXIT:         Not important.
 **
 **/
m355strategy(bp)
register struct buf *bp;
{
register struct unit *un;
register int unit, s;
register u_char cmd;
register struct ctlr *c = &tpctlrs;
register int dmaddr;

	DIAG("TPSTRATEGY: bp=%x\n",*bp);
	DIAG("   b_flags=%x, b_count=%x, b_dev=%x, b_un.b_addr=%x\n"
		,bp->b_flags
		,bp->b_bcount
		,bp->b_dev,bp->b_un.b_addr);
	DIAG("   b_count=%x, b_blkno=%x, b_error=%x\n"
		,bp->b_bcount
		,bp->b_blkno
		,bp->b_error);
    
	unit = TPUNIT(bp->b_dev);
	un = &tpunit[unit];
	if ((unit >= NTP) || (!un->un_present)) {
		bp->b_flags |= B_ERROR;
		DIAG("***exiting m355strategy with error\n");
		iodone(bp);
		return;
	}
	/*
	* Begin critical section
	*/
	s = spl6();
	
	tpcstart(bp);                /* controller start */
	iowait(bp);
	splx(s);			/* End critical section */

	DIAG("---exiting m355strategy without error\n");
}


/**  **/
/**
 ** tpcstart() ---DRIVER SUPPORT ROUTINE
 **
 ** FUNCTION:  Determine read or not-read, then start up
 **            operation on controller.
 **
 ** ENTRY:     mc  -  Pointer to mb_ctlr structure
 **
 ** EXIT:      Not important
 **
 **/
tpcstart(bp)
register struct buf *bp;
{
register struct unit *un;
register struct ctlr *c = &tpctlrs;
register unsigned int dmaddr, unit;

	DIAG("TPCSTART: bp=%x,\n",bp);
	/* 
	* Mark controller busy, and
	* determine destination of this request.
	*/

	unit = TPUNIT(bp->b_dev);
	un = &tpunit[unit];

	if (bp->b_flags & B_READ)
		c->c_cmd = C_RDBLK;
	else
		c->c_cmd = C_WRTBLK;


	c->c_bp     = bp;
	c->c_dmaddr = (u_long) bp->b_un.b_addr;
	c->c_unit   = unit;
	c->c_byterecd = (u_long)bp->b_bcount;
	c->c_recdcnt = 1;
	c->c_fmkcnt  = 0;
	c->c_memtype = MT_XXBIT;
	c->c_addrmod = AM_SD_ND;

	c->c_wantint = TRUE;;

	null_data = FALSE;
	tpcmd(c);
	DIAG("---exiting tpcstart without error\n");
}
/**************************************************************************
 Physical I/O Breakup routine
 */
m355breakup(bp)
register struct buf *bp;
{
	/*
		dma_breakup will check the count against a sector size
		and will abort the operation if not a multiple
		of sector size,
		so dma_breakup is fooled into thinking the
		block is 1 byte long, it does count & (size - 1), which should
		evaluate to 0. This allows the tape controller to 
		check to see if it can handle a particular block size.
	*/

	dma_breakup(m355strategy, bp, 1);
}

/**
 ** m355read()  ---Do reads in the raw.
 **
 **/
m355read(dev)
u_int dev;
{
register struct ctlr *c = &tpctlrs;
register int unit;
register struct unit *un; 

	DIAG("TPREAD: dev=%x\n",dev);

    	unit = TPUNIT(dev); 
	un   = &tpunit[unit];
	if (unit >= NTP) {
		DIAG("***exiting m355read with unit error \n");
		u.u_error = ENXIO;
		return;
	}
	un->un_opened = FREAD;
	tprque(c); 
	DIAG("\n---exiting m355read with out error\n");

	physio(m355breakup,0,dev,B_READ);

	/* u.u_error indicates an error returned from dma_breakup  */
	/*
		If dma_breakup produces an error, the c->c_que flag is
		set because of the call to tprque. It must
		be reset or the entry to close will
		stop on the c->c_que flag
	*/
	if(u.u_error)
		if(c->c_que)
			c->c_que = FALSE;
	return;
}

/**
 ** m355write()  ---Do writes in the raw.
 **
 **/
m355write(dev)
dev_t dev;
{
register struct ctlr *c = &tpctlrs;
register int unit;
register struct unit *un; 

	DIAG("TPWRITE: \n");
	DIAG("TPWRITE: dev=%x\n",dev);

    	unit = TPUNIT(dev); 
	un   = &tpunit[unit];
	if (unit >= NTP) {
		DIAG("***exiting m355write with unit error\n");
		u.u_error = ENXIO;
		return;
	}
	tprque(c);  
	un->un_opened = FWRITE;
	DIAG("---exiting m355write without error\n");

	physio(m355breakup,0,dev,B_WRITE);

	/* u.u_error indicates an error returned from dma_breakup  */
	/*
		If dma_breakup produces an error, the c->c_que flag is
		set because of the call to tprque. It must
		be reset or the entry to close will
		stop on the c->c_que flag
	*/
	if(u.u_error)
		if(c->c_que)
			c->c_que = FALSE;
	return;
}

/**
 ** m355print()
 **
 **/
m355print(dev,str)
register dev;
{
	DIAG("\n%s on m355 unit %d",str,dev);
}

/**
 ** tpwait()  ---?
 **
 **/
tpwait(c)
register struct ctlr *c;
{       
register struct iopb *ip = (struct iopb *)c->c_iopb;
register int deadman;

	deadman = 0;
	DIAG("TPWAIT: c=%x\n",c);
	while (CSR(0) & CS_BUSY) {
		DIAG(" waiting status loop---deadman=%x\r",deadman);
		tpdelay(30);
		if (!(deadman % 0x1000)) DIAG("%x, CSR0=%x  \r",deadman,CSR(0));
		if (++deadman > 0x10000) break;
	}
	DIAG("\n");
	DIAG(" status ok or deadmaned out\n");
	CSR(0) &= ~CS_DONE;

	if (ip->pb_stat == ST_CP_ER) {
		DIAG("***tpwait has error=%x\n",ip->pb_er_cd);
		DEBUG dumpuib(c->c_uib);
		return(ip->pb_er_cd);
	}
	DIAG("---exiting tpwait without error\n");
	return(OK);
}

/**
 ** m355intr()  ---KERNEL DRIVER INTERRUPT HANDLER
 **
 ** FUNCTION: The one and only interrupt (from command completion) handler.
 **
 ** 
 ** EXIT:     Not important.
 **
 **/
m355intr()
{

register struct iopb *ip ;
register struct ctlr *c = &tpctlrs;
register struct buf *bp;
register struct unit *un;
register int unit;
register int i, error_index, error_found = FALSE;

	DIAG("TPINTR: entered, CSR(0)=%x, null_data=%x\n",CSR(0),null_data);
	
	if (!c->c_present)   return;
	
	if ( !(CSR(0) & CS_DONE) ){
	DIAG("TPINTR: entered status change side, SCR=%x, null_data=%x\n"
			,SCR,null_data);
		unit = SCR & SCR_UNMSK;
		un = &tpunit[ unit ];
		if( un->un_rewind ){ 
			DIAG("TPINTR: entered rewind status change side,\n");
			un->un_rewind=FALSE; 
 			un->un_busy = FALSE;
			wakeup((caddr_t)&un->un_busy);
			null_data = FALSE;
		}
		SCR = 0;
		return;
	}

	if (!c->c_wantint)   return;
	if ( !(CSR(0) & CS_ER_LC) ) {		/* no error else error */
		if ( null_data ) {
			un = &tpunit[IOPBUNIT(0)];
			if( c->c_cmd == C_REWQCK ){
				un->un_rewind=TRUE;
				CSR(0) ^= CS_DONE;
				c->c_que = FALSE;
				if(c->c_qued){
					c->c_qued = FALSE;
					wakeup( &c->c_que );
				}
				return;
			}
			un->un_rewind=FALSE;
			un->un_busy = FALSE;
			wakeup((caddr_t)&un->un_busy);
			null_data = FALSE;
		} else {
			bp = c->c_bp; 
			bp->b_resid = 0;
			iodone(bp); 
			null_data = FALSE;
		}
		CSR(0) ^= CS_DONE;
		c->c_que = FALSE;
		if(c->c_qued){
			c->c_qued = FALSE;
			wakeup( &c->c_que );
		}
		return;
	} else {  
		DIAG("TPINTR: entered(error side), CSR(0)=%x, null_data=%x\n"
			,CSR(0),null_data);
		ip = (struct iopb *)c->c_iopb;

		if( ip->pb_er_cd == ER_RD_ST){
			DIAG("m355: short block read error\n");
			bp = c->c_bp; 
			bp->b_resid = (ip->pb_hnotdma << 16)+ip->pb_lnotdma;
			iodone(bp); 
			null_data = FALSE;
			CSR(0) ^= CS_DONE;
			CSR(0) |= CS_SLED;  /* set LED to green  */
			c->c_que = FALSE;
			if(c->c_qued){
				c->c_qued = FALSE;
				wakeup( &c->c_que );
			}
			return;
		}else{
			if( ip->pb_er_cd == ER_FILEMK) {
				un = &tpunit[IOPBUNIT(0)];
				un->un_crfm = TRUE;
				bp = c->c_bp; 
				bp->b_resid = bp->b_bcount;
				iodone(bp); 
				null_data = FALSE;
				CSR(0) ^= CS_DONE;
				CSR(0) |= CS_SLED;	/* set LED to green */
				c->c_que = FALSE;
				if(c->c_qued){
					c->c_qued = FALSE;
					wakeup( &c->c_que );
				}
				return;
			}else{
				if( ip->pb_er_cd == ER_EOT ) {
					bp = c->c_bp; 
					bp->b_resid = bp->b_bcount;
					bp->b_flags |= B_ERROR;
					iodone(bp); 
					null_data = FALSE;
					CSR(0) ^= CS_DONE;
					CSR(0) |= CS_SLED;
							/* set LED to green */
					c->c_que = FALSE;
					if(c->c_qued){
						c->c_qued = FALSE;
						wakeup( &c->c_que );
					}
					return;
				}
			}
		}

		m355elog(c);

		if ( null_data ) {
			un = &tpunit[IOPBUNIT(0)];
			un->un_flags |= B_ERROR;
			un->un_busy = FALSE;
			wakeup((caddr_t)&un->un_busy);
			null_data = FALSE;
		} else {
			bp = c->c_bp; 
			bp->b_resid = 0;
			
			bp->b_flags |= B_ERROR;
			bp->b_error = ENXIO;
			iodone(bp); 
			null_data = FALSE;
		}
		for ( i=0; i<N_KN_ERR; i++) {
			if ( ip->pb_er_cd == errknown[i] ) {
				error_index = i;
				error_found = TRUE;
			}
		}
		if (error_found)
			DIAG("\nm355: cmd=%x, status=%x, error=%x, %s\n"
				,ip->pb_cmd
				,ip->pb_stat
				,ip->pb_er_cd
				,err[error_index]);
		else
			DIAG("\nm355: cmd=%x, status=%x, error=%x, %s\n"
				,ip->pb_cmd
				,ip->pb_stat
				,ip->pb_er_cd
				,err[N_KN_ERR]); 

		dumppb(ip);
		c->c_que = FALSE;
		if(c->c_qued){
			c->c_qued = FALSE;
			wakeup( &c->c_que );
		}
		c->c_wantint = TRUE;		/* restore interrupt driven */
		CSR(0) ^= CS_DONE;
		CSR(0) |= CS_SLED;              /* set LED to green         */
	}
}
/*=========================<  Support routines  >==========================*/

/**  **/
/**
 ** tpcmdslp()  ---DRIVER SUPPORT ROUTINE
 **
 ** FUNTION: Issues command and sleeps on completion.  Returns
 **          error status.
 **
 ** ENTRY:   c  -  Pointer to cntlr structure.
 **
 ** EXIT:    0  -  Command completion without errors.
 **          1  -  Command completion -with- errors.
 **
 **/
tpcmdslp(c)
register struct ctlr *c;
{
register struct unit *un;
register int s;

	DIAG("\nTPCMDSLP: entered\n");

	c->c_wantint = TRUE;
	un = &tpunit[c->c_unit];

	s = spl6();

	tpcmd(c);
	un->un_busy = TRUE;

	while (un->un_busy)
		sleep((caddr_t)&un->un_busy, TPPRI);

	(void)splx(s);
	if (un->un_flags & B_ERROR) {
		un->un_flags &= ~B_ERROR;
		return(NOK);
	}
	un->un_flags &= ~B_ERROR;
	DIAG("\n---exiting tpcmdslp\n");
	return(OK);
} 
/**
 ** tpcmd    ---
 ** put a command out to vtape
 **/
tpcmd(c)
register struct ctlr *c;
{
register struct iopb *ip = (struct iopb *)c->c_iopb;

	DIAG("TPCMD: entered, c=%x\n",c);

	if (CSR(0) & CS_DONE) {
		DIAG("m355: tape driver lost an interrept\n");
		CSR(0) ^= CS_DONE;
	}
	if (c->c_wantint)
		ip->pb_coptn = ( CO_INTEN | (c->c_unit & 0x07) );
	else
		ip->pb_coptn = c->c_unit & 0x07;

	ip->pb_cmd		= c->c_cmd;
	ip->pb_stat		= ip->pb_er_cd = 0;
	ip->pb_hb_rec		= (u_short) (c->c_byterecd >> 16);
	ip->pb_lb_rec		= (u_short)c->c_byterecd;
	ip->pb_rec_c		= c->c_recdcnt;
	ip->pb_fm_cnt		= c->c_fmkcnt;
	ip->pb_hbuf_addr	= (u_short) (c->c_dmaddr >> 16);
	ip->pb_lbuf_addr	= (u_short)c->c_dmaddr;
	ip->pb_dma_burst	= DMA_BURST;
	ip->pb_m_type 		= c->c_memtype;
	ip->pb_adr_mod		= c->c_addrmod;
	ip->pb_int_level	= c->c_lev;
  
	DEBUG dumppb(c->c_iopb);

	DIAG("======> go command\n");
	CSR(0) ^= CS_GO;
	DIAG("---exiting tpcmd\n");
}
/*
 * tpcmdstat() status driven commands
 */
tpcmdst(c)
register struct ctlr *c;
{
	c->c_wantint=FALSE;
	tpcmd(c);
	return(tpwait(c));
}

/*
 * dumppb
 *
 */
dumppb(p)
register struct iopb *p;
{
	DIAG("----VTAPE-IOPB----\n");
DIAG("cmd=%x, cmdopt=%x, stat=%x, err=%x, hbrecd=%x, lbrecd=%x\n"
		,p->pb_cmd
		,p->pb_coptn
		,p->pb_stat
		,p->pb_er_cd
		,p->pb_hb_rec
		,p->pb_lb_rec);

DIAG(" rcnt=%x, baddr=%x, lbaddr=%x\n"
		,p->pb_rec_c
		,p->pb_hbuf_addr
		,p->pb_lbuf_addr);
 
DIAG("mtyp=%x, mmode=%x, ilev=%x, dmab=%x, nvec=%x, evec=%x, fcnt=%x\n"
		,p->pb_m_type
		,p->pb_adr_mod
		,p->pb_int_level
		,p->pb_dma_burst
		,p->pb_n_vec
		,p->pb_e_vec
		,p->pb_fm_cnt);

DIAG(	"r1=%x, hnotdma=%x, lnotdma=%x, notrec=%x, r2=%x\n"
		,p->pb_rv1
		,p->pb_hnotdma
		,p->pb_lnotdma
		,p->pb_notrec
		,p->pb_rv2);

tpdelay(0xffff);
}

/*
 * dumpuib
 */
dumpuib(ub)
register struct uib *ub;
{
	DIAG("----VTAPE----UIB----\n");
	DIAG("14=%x, 16=%x, 36=%x, 44=%x, 26=%x, 50=%x, retry=%x\n"
		,ub->uib_J114
		,ub->uib_J116
		,ub->uib_J136
		,ub->uib_J144
		,ub->uib_J226
		,ub->uib_J250
		,ub->uib_retry
	);
	DIAG("attrib=%x, fill=%x, lrew=%x, vrew=%x, l=%x, v=%x\n"
		,ub->uib_attr
		,ub->uib_fill
		,ub->uib_lv_rw
		,ub->uib_vc_rw
		,ub->uib_lev
		,ub->uib_vec
	);
}
/*
 *	timer kluge-- counts down to zero, assumes usigned count
 */
tpdelay(i)
register u_long i;
{
	for(;i>0;i--);
}

/**  **/
/*
 * unitinit()  ---DRIVER SUPPORT ROUTINE
 *
 * FUNCTION: Set up initialize parameters here.
 *           
 * ENTRY:    c  -  Pointer to the cntlr structure.
 *
 * EXIT:     Not important.
 *
 */
unitinit(c)
register struct ctlr       *c;
{
register struct uib *dfub;
register struct uib *ub;
register int unit = c->c_unit;

	DIAG("UNITINIT: entered, c=%x\n",*c);
	/*
	* initialize unit
	*/
	DIAG("   c->c_uib=%x\n",c->c_uib);

	ub = (struct uib *)c->c_uib;
	dfub = &default_uib;

	ub->uib_J114       = dfub->uib_J114;
	ub->uib_J116       = dfub->uib_J116;
	ub->uib_J136       = dfub->uib_J136;
	ub->uib_J144       = dfub->uib_J144;
	ub->uib_J226       = dfub->uib_J226;
	ub->uib_J250       = dfub->uib_J250;
	ub->uib_rsv1       = dfub->uib_rsv1;
	ub->uib_rsv2       = dfub->uib_rsv2;
	ub->uib_rsv3       = dfub->uib_rsv3;
	ub->uib_rsv4       = dfub->uib_rsv4;
	ub->uib_rsv5       = dfub->uib_rsv5;
	ub->uib_retry      = dfub->uib_retry;
	ub->uib_rsv6       = dfub->uib_rsv6;
	ub->uib_rsv7       = dfub->uib_rsv7;
	ub->uib_attr       = dfub->uib_attr;
	ub->uib_fill       = dfub->uib_fill;
	ub->uib_lv_rw      = dfub->uib_lv_rw;
	ub->uib_vc_rw      = dfub->uib_vc_rw;
	ub->uib_lev        = dfub->uib_lev;
	ub->uib_vec        = dfub->uib_vec;       

	c->c_cmd      = C_INIT;
	c->c_dmaddr   = (u_long) c->c_uib;
	c->c_recdcnt  = 1;
	c->c_fmkcnt   = 0;
	c->c_byterecd = 0x1000;
	c->c_memtype  = MT_I16BIT;
	c->c_addrmod  = AM_ST_N;

	DEBUG dumpuib(c->c_uib);
	return;
}

/**  **/
/*
 * tpswab()  ---DRIVER SUPPORT ROUTINE
 *
 * FUNCTION: Routine to set up byte swap.
 *
 * ENTRY:
 *
 * EXIT:
 *
 */
tpswab(c)
register struct ctlr *c;
{
register struct uib *ub;
	
	DIAG("TPSWAB: entered\n");

	ub = (struct uib *)c->c_uib;
	ub->uib_attr |= 0x4;

	DIAG("---exiting tpswab\n");
}


/*
 * tphiden()  ---DRIVER SUPPORT ROUTINE
 *
 * FUNCTION:  Routine to select high density.
 *            Very drive specific.
 *
 * ENTRY:
 *
 * EXIT:
 *
 */
tphiden(c)
register struct ctlr *c;
{
register struct uib *ub;
	
	DIAG("TPHIDEN: entered\n");

	ub = (struct uib *)c->c_uib;
	ub->uib_J136 = 1;

	DIAG("---exiting tphiden\n");
}


/*
 * tphispd()  ---DRIVER SUPPORT ROUTINE
 *
 * FUNCTION:  Routine to select high speed.
 *            Very drive specific.
 *
 * ENTRY:
 *
 * EXIT:
 *
 */
tphispd(c)
register struct ctlr *c;
{
register struct uib *ub;
	
	DIAG("TPHISPD: entered\n");

	ub = (struct uib *)c->c_uib;
	ub->uib_J250 = 1;

	DIAG("---exiting tphispd\n");
}

/*
 * tprque()  ---DRIVER SUPPORT ROUTINE
 *
 * FUNCTION: To block the command when the controller is busy 
 *
 * ENTRY:	should be call before driver changes clr struct
 *
 * EXIT:	returns when the controller is free
 *
 */
tprque(c)
register struct ctlr *c;
{
register int s;

	DIAG("TPRQUE: entered c->c_que==%x\n",c->c_que);
	/*
	* Begin critical section
	*/
	s = spl6();
	
	while(c->c_que){
		c->c_qued = TRUE;
		sleep( &c->c_que,TPPRI );
	}
	c->c_que = TRUE;

	(void)splx(s);			/* End critical section */
}

/*
 * u_st_init()  ---DRIVER SUPPORT ROUTINE
 *
 * FUNCTION: init the unit struct.
 *
 *
 * EXIT:	
 *
 */
u_st_init(unit,y_n)
register int unit,y_n;
{
static struct  unit *up;
	up=&tpunit[unit];

        if(y_n){
		up->un_present = TRUE;	  /* physical unit is present */
        	up->un_opened  = FALSE;    /* lock against multiple opens */
        	up->un_busy    = FALSE;
	}else{
		up->un_present = FALSE;	  /* physical unit is present */
        	up->un_opened  = TRUE;     /* lock against multiple opens */
        	up->un_busy    = TRUE;
	}

}

/* fake iobuf to force error logging */
struct iobuf iob = {0};
unsigned short io_addr[2] = 0;

m355elog(c)
register struct ctlr *c;
{
	register struct iopb *ip  = c->c_iopb;
	register struct buf *bp = c->c_bp;
	register struct iobuf *dp = &iob;
	register long status;


	dp->io_addr = io_addr;
	io_addr[0] = ip->pb_cmd;
	io_addr[1] = ip->pb_er_cd;
	dp->io_stp = 0;
	dp->b_actf = bp;
	dp->io_nreg = 2; 	/* in words */
	dp->b_dev = makedev(M355, minor(bp->b_dev));
	status = ip->pb_er_cd;
	
	fmtberr(dp, 0); /* format a block error */
	logberr(dp, status); /* log a block error */

}

