 /*                                                                            *
 *  attach.c         attach()           1.0                                   *
 *  buf_access.c     buf_to_iopb()      1.0                                   *
 *                   update_buf()       1.0                                   *
 *  close.c          m323close()        1.0         UNIX-defined              *
 *  commands.c       copy()             1.0                                   *
 *                   format()           1.0                                   *
 *                   get_table()        1.0                                   *
 *                   header_data()      1.0                                   *
 *                   read_parms()       1.0                                   *
 *                   set_parms()        1.0                                   *
 *  displays.c       dump_iopb()        1.0         Debug Routine             *
 *  execute.c        execute()          1.0                                   *
 *                   poll()             1.0                                   *
 *  go.c             go()               1.0                                   *
 *  interrupt.c      m323intr()         1.0         UNIX-defined              *
 *  ioctl.c          m323ioctl()        1.0         UNIX-defined              *
 *  iopb_access.c    get_addr()         1.0                                   *
 *                   init_iopb()        1.0                                   *
 *                   make_checksum()    1.0                                   *
 *                   make_iopb()        1.0                                   *
 *                   put_addr()         1.0                                   *
 *                   valid_iopb()       1.0                                   *
 *  logical_blk.c    convert_blkno()    1.0                                   *
 *  open.c           m323open()         1.0         UNIX-defined              *
 *  print.c          m323print()        1.0         UNIX-defined              *
 *  queue_access.c   deq_buf()          1.0                                   *
 *                   deq_iopb()         1.0                                   *
 *                   enq_buf()          1.0                                   *
 *                   enq_iopb()         1.0                                   *
 *                   flush_queue()      1.0                                   *
 *                   init_queues()      1.0                                   *
 *  read.c           m323read()         1.0         UNIX-defined              *
 *  reg_access.c     clear_bit()        1.0                                   *
 *                   read_addr()        1.0                                   *
 *                   read_reg()         1.0                                   *
 *                   set_bit()          1.0                                   *
 *                   stuff_addr()       1.0                                   *
 *                   test_bit()         1.0                                   *
 *                   write_reg()        1.0                                   *
 *  strategy.c       m323strategy()     1.0         UNIX-defined              *
 *  write.c          m323write()        1.0         UNIX-defined              *
 *                                                                            *
 ******************************************************************************

            ****************************************
            *                                      *
            *  F U N C T I O N - T O - F I L E     *
            *   C R O S S - R E F E R E N C E      *
            *                                      *
            *  attach()            attach.c        *
            *  buf_to_iopb()       buf_access.c    *
            *  clear_bit()         reg_access.c    *
            *  convert_blkno()     logical_blk.c   *
            *  copy()              commands.c      *
            *  deq_buf()           queue_access.c  *
            *  deq_iopb()          queue_access.c  *
            *  dump_iopb()         displays.c      *
            *  enq_buf()           queue_access.c  *
            *  enq_iopb()          queue_access.c  *
            *  execute()           execute.c       *
            *  flush_queue()       queue_access.c  *
            *  format()            commands.c      *
            *  get_addr()          iopb_access.c   *
            *  get_table()         commands.c      *
            *  go()                go.c            *
            *  header_data()       commands.c      *
            *  init_iopb()         iopb_access.c   *
            *  init_queues()       queue_access.c  *
            *  make_checksum()     iopb_access.c   *
            *  make_iopb()         iopb_access.c   *
            *  poll()              execute.c       *
            *  put_addr()          iopb_access.c   *
            *  read_addr()         reg_access.c    *
            *  read_parms()        commands.c      *
            *  read_reg()          reg_access.c    *
            *  set_bit()           reg_access.c    *
            *  set_parms()         commands.c      *
            *  stuff_addr()        reg_access.c    *
            *  test_bit()          reg_access.c    *
            *  update_buf()        buf_access.c    *
            *  valid_iopb()        iopb_access.c   *
            *  write_reg()         reg_access.c    *
            *  m323close()         close.c         *
            *  m323init()          init.c          *
            *  m323intr()          interrupt.c     *
            *  m323ioctl()         ioctl.c         *
            *  m323open()          open.c          *
            *  m323print()         print.c         *
            *  m323read()          read.c          *
            *  m323strategy()      strategy.c      *
            *  m323write()         write.c         *
            *                                      *
            ****************************************/

#include "sys/param.h"
#include "sys/types.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/buf.h"
#include "sys/iobuf.h"
#include "sys/systm.h"
#include "sys/sysmacros.h"
#include "sys/elog.h"
#include "sys/open.h"
#include "sys/dk.h"
#include "sys/cmn_err.h"
#include "sys/mvme323.h"

/* #define DEBUG */ 		/* display debug messages */ 
#define AUTOCONFIG   	/* enable auto configuration */

#define CCNT M323CTLS                 /* CONTROLLER COUNT */
#define UCNT M323DRV                  /* UNIT COUNT       */
#define PCNT M323SLICE                /* PARTITION COUNT  */
#define ICNT M323ICNT                 /* IOPB COUNT       */
#define CTL CTLR
#define BUSY CBUSY

extern REGISTERS *m323_addr[];   /* Controller Addresses */             
extern int m323_cnt;            /* Number of Controllers In System */
extern int m323_ivec[];         /* Vector of Interrupt Vector Addresses */
extern int m323_ilev[];         /* Vector of Interrupt Levels */
extern int spl1(), spl2(), spl3(), spl4(), spl5(), spl6(), spl7();
extern struct m323size part_table[];
extern struct iobuf m323utab[];
extern struct iotime m323stat[];
extern struct m323eregs m323eregs[];
extern UNITINFO unit_table[];
extern FORMATINFO format_parm;
extern CTLRINFO ctlr_table;
extern int m323otbl[];
extern IOPBPTR iopb[];
extern IOPBPTR riopb[];
extern IOPB io_vec[];
extern BUF rbuf[];
extern short m323rcnt;
extern short m323qcnt;
extern char  m323cflush;


LOCAL USHORT  autoconf = 0;		/* adjust drive parameters at open */
LOCAL USHORT  Major_Rev = 2;          /* Major Software Revision Number.      */
LOCAL USHORT  Minor_Rev = 1;          /* Minor Software Revision Number.      */

LOCAL USHORT  Ctlr_Type = 7;          /* 421 = 0, 431 = 1, 432 = 2, 450 = 3   */
                                      /* 450E = 4, 451 = 5, 751 = 6, 712 = 7  */

LOCAL BYTE    Ctype = 0x12;           /* Ctlr Type Used in Set Ctlr Par Cmd   */

LOCAL long    LOOP_LIMIT = 20000;

LOCAL USHORT  Retries = 5;            /* Default Retry Count */
LOCAL BOOL    Debug = NO;


/* Global structures and variables. */

LOCAL QUEUE   Queue;                  /* Queue structure head.                */
LOCAL IOPB    free_head;              /* Head node of FREE IOPB queue.        */
LOCAL IOPB    ready_head;             /* Head node of READY IOPB queue.       */
LOCAL BUF     buf_head;               /* Head node of BUF queue.              */
LOCAL BUF     read_buf;               /* Buf structure for read().            */
LOCAL BUF     write_buf;              /* Buf structure for write().           */
 
LOCAL char    work_buffer[WORK_BUFF_SIZE];
LOCAL BOOL    Boot_Flag = YES;        /* Initialization flag.                 */
LOCAL short   optim_cnt;	      /* down counter which counts iopbs sent
				         to controller. */

typedef struct
    {
    int (*function)();
    } INTR_LOCK;

LOCAL INTR_LOCK intr_lock[7] =
			{
			spl1,
			spl2,
			spl3,
			spl4,
			spl5,
			spl6,
			spl7,
			};



       /* GENERIC DISK COMMANDS AND SUBFUNCTIONS */
                  /* NO OPERATION */

#define Nop             0x0

                    /* WRITE */

#define Write           0x1

                    /* READ */

#define Read            0x2

                    /* SEEK */

#define Seek            0x3

#define Report          0x00  /* Report Current Address */
#define Seek_Report     0x01  /* Seek & Report Current Address */
#define Seek_Start      0x02  /* Seek Start */

                    /* RESET */

#define Drive_Reset     0x4   /* Drive Reset */

               /* WRITE PARAMETERS */

#define Write_Par       0x5

#define W_Controller    0x00  /* Write Control Parameters */
#define W_Drive         0x80  /* Write Drive Parameters */
#define W_Format        0x81  /* Write Controller Format Parameters */

               /* READ PARAMETERS */

#define Read_Par        0x6

#define R_Controller    0x00  /* Read Control Parameters */
#define R_Drive         0x80  /* Read Drive Parameters */
#define R_Format        0x81  /* Read Controller Format Parameters */

               /* EXTENDED WRITE */

#define Ext_Write       0x7

#define W_Trk_Hdr       0x80  /* Write Track Headers */
#define W_Format        0x81  /* Write Format */
#define W_HDE           0x82  /* Write Hdr, Hdr ECC, Data & Data ECC */

               /* EXTENDED READ */

#define Ext_Read        0x8

#define R_Trk_Hdr       0x80  /* Read Track Headers */
#define RW_Check        0x81  /* Read/Write Check Data (Verify) */
#define R_HDE           0x82  /* Read Hdr, Hdr ECC, Data & Data ECC */

               /* ABORT COMMAND */

#define Abort           0xa

               /* Controller Specific Subfunctions */

#define R_Status	0xb0  /* Read Drive Status Extended */
#define W_Def		0xb0  /* Write Defect Map */
#define R_Def		0xb0  /* Read Defect Map */
	
#define Fault_Clear	0x80  /* Fault Clear */
#define St_Fmt_Conf	0xb0  /* Store Format Configuration */
#define Drv_Conf	0xb1  /* Show Drive Configuration */

               /* SEND OPTIONAL COMMAND */

#define Opt_Cmd         0xc   /* Send Optional Command */

#define No_Status       0xb0  /* Send Optional Command */
#define Exp_Status	0xb1  /* Expect Status */



/*******************************************************************************

DESCRIPTION:
		Definitions of the byte offsets to
		the various fields within the IOPB.
                                                                            
******************************************************************************/ 

/* IOPB byte offsets. */

#define COMMAND        0x00          /* Command byte.                  */
#define COMPCODE       0x01          /* Completion codes.              */
#define DISKSTAT       0x02          /* Disk status.                   */
#define STAT3          0x03          /* Status 3 byte.                 */
#define SUBFUNC        0x04          /* Subfunction byte.              */
#define FMTPAR         0x04          /* Format parameters.             */
#define UNIT           0x05          /* Unit number.                   */
#define LEVEL          0x06          /* Interrupt level.               */
#define DRV_PARM       0x06          /* Drive parameters modification. */
#define INTERLEAVE     0x06          /* Format parameters interleave.  */
#define VECTOR         0x07          /* Interrupt vector.              */
#define COUNTH         0x08          /* Byte count high.               */
#define CTLRPAR1       0x08          /* Controller parameters 1.       */
#define MAXSECLH       0x08          /* Max. sectors on last head.     */
#define FLD_1          0x08          /* Format parameters field 1.     */
#define FLD1H          0x08          /* Field 1 high byte.             */
#define COUNTL         0x09          /* Byte count low.                */
#define CTLRPAR2       0x09          /* Controller parameters 2.       */
#define HDOFFSET       0x09          /* Head offset.                   */
#define FLD_2          0x09          /* Format parameters field 2.     */
#define FLD1L          0x09          /* Field 1 low byte.              */
#define CYLH           0x0a          /* Cylinder number high.          */
#define CTLRPAR3       0x0a          /* Controller parameters 3.       */
#define FLD2H          0x0a          /* Field 2 high byte.             */
#define FLD_3          0x0a          /* Format parameters field 3.     */
#define CYLL           0x0b          /* Cylinder number low.           */
#define THROTTLE       0x0b          /* Controller parms throttle.     */
#define FLD2L          0x0b          /* Field 2 low byte.              */
#define FLD_4          0x0b          /* Format parameters field 4.     */
#define HEAD           0x0c          /* Head number.                   */
#define FLD3H          0x0c          /* Field 3 high byte.             */
#define FLD_5H         0x0c          /* Format parameters field 5 Hi.  */
#define SECTOR         0x0d          /* Sector number.                 */
#define FLD3L          0x0d          /* Field 3 low byte.              */
#define FLD_5L         0x0d          /* Format parameters field 5 Lo.  */
#define DATA_MOD       0x0e          /* Data address modifier.         */
#define ACT_SECTORS    0x0e          /* Actual no. of sectors/track.   */
#define FLD_12         0x0e          /* Format parameters field 12.    */
#define IOPB_MOD       0x0f          /* IOPB address modifier.         */
#define DATA1          0x10          /* Data address byte 1.           */
#define FLD4H          0x10          /* Field 4 high byte.             */
#define FLD_6          0x10          /* Format parameters field 6.     */
#define PART_HI        0x10          /* Part number high byte.         */
#define DATA2          0x11          /* Data address byte 2.           */
#define FLD4L          0x11          /* Field 4 low byte.              */
#define FLD_7          0x11          /* Format parameters field 7.     */
#define PART_LO        0x11          /* Part number low byte.          */
#define DATA3          0x12          /* Data address byte 3.           */
#define FLD5H          0x12          /* Field 5 high byte.             */
#define FLD_5AH        0x12          /* Format parameters field 5 Alt Hi*/
#define REVISION       0x12          /* Revision number of controller. */
#define DATA4          0x13          /* Data address byte 4.           */
#define FLD5L          0x13          /* Field 5 low byte.              */
#define FLD_5AL        0x13          /* Format parameters field 5 Alt Lo*/
#define SUBREV         0x13          /* Sub-revision number of ctlr.   */
#define NIOPB1         0x14          /* Next IOPB address byte 1.      */
#define NIOPB2         0x15          /* Next Iopb address Byte 2.      */
#define NIOPB3         0x16          /* Next Iopb address Byte 3.      */
#define NIOPB4         0x17          /* Next Iopb address Byte 4.      */
#define CHECKH         0x18          /* Checksum high byte.            */
#define ALTFLDH        0x18          /* Alternate field high.          */
#define CHECKL         0x19          /* Checksum low byte.             */
#define ALTFLDL        0x19          /* Alternate field low.           */
#define ECCPH          0x1a          /* ECC pattern word high.         */
#define ECCPL          0x1b          /* ECC pattern word low.          */
#define ECCOH          0x1c          /* ECC offset word high.          */
#define ECCOL          0x1d          /* ECC offset word low.           */

/* b_flags constants */
#define B_ABS   0x010000	/* do not add partition offset to b_blkno */
#define B_COPEN 0x020000	/* open in progress */
#define	B_CONF	0x040000	/* Need to replace default config info */
				/* with config info stored on disk. */

/* identify all non read/write I/O commands for correct I/O time accounting */  
#define NON_RWIO  B_CONF


/******************************************************************************
FILE = %W%	%G%	Inc.
*******************************************************************************
TITLE:		ATTACH()
*******************************************************************************

DESCRIPTION:
		This function attempts to bring a Unit on line by issuing five
		IOPBs:
		    1) Drive Reset
		    2) Set Drive Parameters
		    3) Read Drive Parameters
		    4) Set Controller Parameters
		    5) Read Controller Parameters
		    6) Controller Reset
		The driver's internal unit and controller information tables
		will be updated to reflect the results.  The operating system 
		will be informed of the unit being on line or not.
                                                                            
REQUIRES:                                                               
		The unit and controller information tables.

SUBROUTINE CALLS:                                                       
		execute() - Executes an IOPB in a polled fashion.
		dri_printf() - Prints message on console device. (system)

EFFECTS:                                                                
		Resets the drive.
		Sets drive and controller parameters.
		Initializes unit and controller information tables.

ROUTINE RETURN VALUES:                                                  
		TRUE - Attach was successfull.
		FALSE - Attach failed.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL attach (unit)
USHORT        unit;
{
    FAST CTLRINFO    *cinfo;
    FAST UNITINFO    *uinfo;
    FAST PARTINFO    *pinfo;
    FAST IOPBPTR     iopbptr;
    FAST USHORT      part, fail_flag = 0;

cinfo = &ctlr_table;        /* Point at the information tables. */
uinfo = &unit_table[unit];
pinfo = (PARTINFO *)&(part_table[(unit * PCNT) + FIRST_PARTITION]);  
    
iopbptr = riopb[unit];    /* Get the raw IOPB associated with this unit. */
    
/* Send a Drive Reset IOPB, a Set Controller Parameters IOPB, Set Drive */
/* Parameters IOPB, and a Read Drive Paramaters IOPB. If they ALL       */
/* return successfully, then store the values returned in the unit      */
/* information table. If ANY of them fail, then mark the unit offline   */
/* and return FALSE.                                                    */
    
if ( execute( POLLED, XYDRVRESET, unit ) IS_NOT SUCCESSFULL )
    fail_flag = XYDRVRESET;
else if ( execute( POLLED, XYSETDRVPAR, unit ) IS_NOT SUCCESSFULL )
    fail_flag = XYSETDRVPAR;
else if ( execute( POLLED, XYRDDRVPAR, unit ) IS_NOT SUCCESSFULL )
    fail_flag = XYRDDRVPAR;
else
    {
    uinfo->heads = iopbptr->byte[HEAD] + 1;
    uinfo->cylinders = ((iopbptr->byte[CYLH] << 8) |
    iopbptr->byte[CYLL]) + 1;
    uinfo->sectors = iopbptr->byte[SECTOR] + 1;

    if ( iopbptr->byte[ACT_SECTORS] > uinfo->sectors )
        uinfo->trk_spares = iopbptr->byte[ACT_SECTORS] - uinfo->sectors;
    else
        uinfo->trk_spares = 0;

    uinfo->drive_parms = iopbptr->byte[DRV_PARM] & INT_LEVEL_MASK;

    if ( unit == 0 ) /* If this is the 1st Unit, execute Controller Cmds */
        {
        if ( execute( POLLED, XYSETCTLRPAR, unit ) IS_NOT SUCCESSFULL )
            fail_flag = XYSETCTLRPAR;
        else if ( execute( POLLED, XYRDCTLRPAR, unit ) IS_NOT SUCCESSFULL )
            fail_flag = XYRDCTLRPAR;
        else
            {
            cinfo->param1 = iopbptr->byte[CTLRPAR1];
            cinfo->param2 = iopbptr->byte[CTLRPAR2];
            cinfo->param3 = iopbptr->byte[CTLRPAR3];
            cinfo->throttle = iopbptr->byte[THROTTLE];
            }

        }
    }
    
write_reg( CSR, RESET );     /* Controller Reset */

if ( fail_flag IS_NOT 0 )    /* An Error Occured */
    {
#ifdef DEBUG
    dri_printf("mvme323 drive %d: Failed ", unit);
    switch( fail_flag )
        {
        case XYDRVRESET:     dri_printf("Drive Reset\n");                break;
        case XYSETDRVPAR:    dri_printf("Set Drive Parameters\n");       break;
        case XYSETCTLRPAR:   dri_printf("Set Controller Parameters\n");  break;
        case XYRDDRVPAR:     dri_printf("Read Drive Parameters\n");      break;
        case XYRDCTLRPAR:    dri_printf("Read Controller Parameters\n"); break;
        default:             dri_printf("For Some Unknown Reason\n");    break;
        }
#endif

    for ( part = 0; part < PCNT; ++part, pinfo++ )  
        pinfo->p_status = ABSENT;

    return(FALSE);
    }
else
    {

#ifdef DEBUG
    /* Send the ON LINE display to the console device. */
    dri_printf("mvme323 drive %d:  cyl %d  hd %d  sec %d  \n", 
    unit, uinfo->cylinders, uinfo->heads, uinfo->sectors);
#endif

    /* Mark all of the unit's partitions PRESENT. */

    for (part = 0; part < PCNT; ++part, pinfo++)  
        pinfo->p_status = PRESENT;
    
    return(TRUE);
    }
}


LOCAL setformparm(ctlr)
USHORT  ctlr;	/* controller */
{
	register FORMATINFO *finfo;
	register IOPBPTR iopbptr;
	USHORT unit;

	unit = 0;	
	iopbptr = riopb[(ctlr * M323DRV) + unit];    /* Get the raw IOPB */
	finfo = &format_parm;
        iopbptr->byte[INTERLEAVE] |= finfo->interleave;
        iopbptr->byte[FLD_1] = finfo->field_1;
        iopbptr->byte[FLD_2] = finfo->field_2;
        iopbptr->byte[FLD_3] = finfo->field_3;
        iopbptr->byte[FLD_4] = finfo->field_4;
        iopbptr->byte[FLD_5H] = finfo->field_5H;
        iopbptr->byte[FLD_5L] = finfo->field_5L;
        iopbptr->byte[FLD_5AH] = finfo->field_5AH;
        iopbptr->byte[FLD_5AL] = finfo->field_5AL;
        iopbptr->byte[FLD_6] = finfo->field_6;
        iopbptr->byte[FLD_7] = finfo->field_7;
        iopbptr->byte[FLD_12] = finfo->field_12;

        if ( execute( POLLED, XYSETFORMPAR, unit ) IS_NOT SUCCESSFULL )
		dri_printf("M323: Failed to set format parameters.\n", unit);
}



/******************************************************************************
FILE = %W%	%G%	Inc.
*******************************************************************************
TITLE:		BUF_TO_IOPB()
*******************************************************************************

DESCRIPTION:
		This function will take buf structures and make IOPB's out of
		them until it runs out of either one of them.  It does this by
		dequeueing a buf structure from the BUF queue and an IOPB from
		the FREE IOPB queue.  It makes an IOPB out of the buf struc-
		ture and queues it onto the READY IOPB queue.  It calls 
		itself before returning in case it can make more IOPBs.  It 
		returns TRUE if it made an IOPB, and FALSE if there were no 
		buf structures or free IOPBs to use.
 
REQUIRES:                                                               
		A buf structure and a FREE IOPB to return successfully.

SUBROUTINE CALLS:                                                       
		make_iopb() - Makes an IOPB out of a buf structure.
		deq_iopb() - Dequeues an IOPB from the FREE IOPB queue.
		deq_buf() - Dequeues a buf from the BUF queue.
		enq_iopb() - Enqueues an IOPB onto a queue.
		buf_to_iopb() - Recursively calls itself.        

EFFECTS:                                                                
		This function acts as an interface between strategy() and
		make_iopb().  It dequeues buf structures and free IOPBs, and
		queues IOPBs onto the READY IOPB queue.

ROUTINE RETURN VALUES:                                                  
		TRUE if at least one IOPB was made and queued onto the 
		READY IOPB queue.
		FALSE if there was either no buf structure or no free IOPB.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL buf_to_iopb()
{
    FAST IOPBPTR iopbptr;
    IOPBPTR      deq_iopb();
    FAST BUFPTR  bufptr;
    BUFPTR       deq_buf();
    register struct iobuf *dp;

    /* If either the BUF queue OR the FREE IOPB */
    /* queue are empty, then return FALSE.      */
    
    if (Queue.buffs->av_forw IS EMPTY) 
        return(FALSE);

    /* Periodically, allow controller to complete all IOPBs in its queue
       before sending it more IOPBs to perform. */
    if(m323cflush)     
	return(FALSE);

    if ((iopbptr = deq_iopb(Queue.free)) IS EMPTY )
        return(FALSE);
    
    bufptr = deq_buf();               /* Dequeue a buf structure. */
    make_iopb(iopbptr, bufptr);       /* Make an IOPB out of it.  */
    iopbptr->flags |= IOPBACTIVE;
    m323qcnt += 1;		      /* count the number of requests enqueued*/
    enq_iopb(Queue.ready, iopbptr);  /* Enqueue it on the READY IOPB queue. */
    
    dp = &m323utab[POS(iopbptr->buffer->b_dev)];
    dp->io_start = lbolt;	/* record the start time */
    buf_to_iopb();       /* Call this routine recursively until */
                         /* no more buf-IOPB pairs can be made. */
    return(TRUE);
}

/******************************************************************************
TITLE:		UPDATE_BUF()
*******************************************************************************

DESCRIPTION:
		This function is responsible for updating the buf structure(s)
		that is/are associated with the IOPB or IOPB chain that is sent
	 	to it.  After doing so, the bufs are returned to the system via
		a call to iodone().

REQUIRES:                                                               
		An IOPB pointer.

SUBROUTINE CALLS:                                                       
		get_addr() - Get the "next" IOPB address from an IOPB.
		update_iopb() - Recursively calls this routine.
		iodone() - Returns a buf structure back to the operating
		    system. (system)

EFFECTS:                                                                
		Updates the b_flags, b_error, b_resid fields of the buf 
		structure that is associated with a completed IOPB.

ROUTINE RETURN VALUES:                                                  
		None.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL update_buf(iopbptr)
register IOPBPTR    iopbptr;
{
    register BUFPTR     bufptr;
    register struct iobuf *dp;
    IOPBPTR    nxt_iopbptr;
    ULONG      get_addr();
    BOOL       retry = NO;
    UINT       unit, errortype;

bufptr = iopbptr->buffer;  
dp = &m323utab[POS(bufptr->b_dev)];
unit = DRIVE(bufptr->b_dev);     
errortype = SOFTERR;

if ( (iopbptr->byte[COMMAND] & ERRS) &&
     ( ( RAWBUF(bufptr) || Debug ||
         INTERVENTION(iopbptr->byte[COMPCODE]) ||
         MISC_ERROR(iopbptr->byte[COMPCODE])  ||
         RESET_RETRY(iopbptr->byte[COMPCODE]) ||
         YOUR_LOST(iopbptr->byte[COMPCODE])   ||
        (RETRY_HARD(iopbptr->byte[COMPCODE]) && bufptr->b_error >= Retries)) ) )
    {

    dri_printf("\nM323: command 0x%x failed on unit %d slice %d  Completion Code 0x%x", 
		iopbptr->byte[COMMAND], unit, PARTITION(bufptr->b_dev), 
		iopbptr->byte[COMPCODE]);

    if ( RETRY_HARD(iopbptr->byte[COMPCODE]) )
        dri_printf("...Retry No. %d\n", bufptr->b_error + 1 );
    else
        dri_printf("\n");
    }

if ( RAWBUF(bufptr) )  /* If raw, copy the IOPB to its associated raw IOPB. */
    copy(iopbptr, bufptr->b_bcount, sizeof(IOPB));
else
    {  
    if ( (iopbptr->byte[COMMAND] & ERRS) &&    /* NON-SOFT ERROR OCCURED?  */
          NOT(SOFT_ERROR( iopbptr->byte[COMPCODE] )) ) 
        {
        if ( RETRY_HARD( iopbptr->byte[COMPCODE] ) && /* Is Retryable?      */
             bufptr->b_error < Retries )              /* Retries Available? */
            {
            ++bufptr->b_error; /* INCREMENT RETRY COUNT */
	    m323elog(iopbptr, dp, errortype);
            retry = YES;
            }
        else         
            {
	    errortype = HARDERR;
            bufptr->b_resid = bufptr->b_bcount;
            bufptr->b_flags |= B_ERROR;
            bufptr->b_error = EIO;
	    m323elog(iopbptr, dp, errortype);
            }
        }
    else             /* NO ERROR */
        {
        bufptr->b_resid = 0;        /* The operation was a success,  */
        bufptr->b_error = CLEAR;    /* so set the flags accordingly. */
        }
    }

/* The following code has been removed because this driver was not designed
   to use the interrupt at end of chain feature in the controller. To 
   effectively use the feature, the driver must maintain a seperate ready queue 
   for each drive so that only commands for the same drive will be chained. 
   The interrupt will occur when all commands in the chain have completed. */
#ifdef 0
if (iopbptr->byte[COMMAND] & CHEN)
    {  
    /* recursively call update_buf for all IOPBs in the chain. */
    nxt_iopbptr = (IOPBPTR)get_addr(iopbptr, NEXT_IOPB);
    update_buf(nxt_iopbptr); 
    }
#endif

if ( retry )
    m323strategy( bufptr );
else
    iodone(bufptr);      /* Signal the world that the buf is done. */
}



/*
 *	m323elog: performs error logging
 */

m323elog(iopbptr, dp, errortype)
register IOPBPTR iopbptr;
register struct iobuf *dp;
register USHORT errortype;
{	
	register struct m323eregs *erp;
	register BUFPTR bufptr;
	register PARTINFO  *pinfo;
	register UINT part, unit;

	bufptr = iopbptr->buffer;
	unit = DRIVE(bufptr->b_dev);     /* Set up local variables and pointers. */
	part = PARTITION(bufptr->b_dev);     
        pinfo = (PARTINFO *)&(part_table[(unit*PCNT) + part]);


	erp = (struct m323eregs *)dp->io_addr;

	erp->er_blkoff = bufptr->b_blkno + pinfo->first; /* block offset */
	erp->er_cmd = iopbptr->byte[COMMAND];   /* iopb command byte */
	erp->er_cmplcode = iopbptr->byte[COMPCODE]; /* iopb completion code */
	erp->er_st2 = iopbptr->byte[DISKSTAT]; /* drive status */
	erp->cyl = (iopbptr->byte[CYLH] << 8) + iopbptr->byte[CYLL];
	erp->head = iopbptr->byte[HEAD];

	/* get major number of block device for error logging */
	dp->b_dev = makedev( M323, minor(dp->b_dev));
	dp->b_actf = bufptr;

	fmtberr( dp, 0 ); 		/* post to error log */

	/* finalize the error report for this error */  
	logberr(dp, errortype);
}




/******************************************************************************
FILE = %W%	%G%	Inc.
*******************************************************************************
TITLE:		CLOSE()
*******************************************************************************

DESCRIPTION:
		This function is called when a device is unmounted, or
		if a special file for this device that was opened explicitly
		is closed.

REQUIRES:                                                               
		A device number.

SUBROUTINE CALLS:                                                       
		None.


ROUTINE RETURN VALUES:                                                  
		None.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

m323close (devnum, flag, otyp)
dev_t    devnum;
int      flag;
int	 otyp;
{
    FAST USHORT      ctl, unit, part;
    FAST UNITINFO   *uinfo;
    FAST PARTINFO   *pinfo;
    FAST int 	    *otbl;	/* address of open table */	     			
    ctl =  CTL(devnum);
    unit = DRIVE(devnum);        /* Set up local variables and pointers. */
    part = PARTITION(devnum);
    uinfo = &unit_table[unit];
    pinfo = (PARTINFO *)&(part_table[(unit * PCNT) + part]);
    otbl = &m323otbl[((ctl * UCNT) + unit) * OTYPCNT];


   /* mark device closed */
   if(otyp == OTYP_LYR)
	--otbl[OTYP_LYR];
   else if (otyp < OTYPCNT)
	otbl[otyp] &= ~( 1 << part);
    
}

/******************************************************************************
FILE = %W%	%G%	Inc.
*******************************************************************************
TITLE:		COPY()
*******************************************************************************

DESCRIPTION:
		This support routine simply copies data from one structure 
    		to another.

REQUIRES:                                                               
		Two character pointers and a byte count.

SUBROUTINE CALLS:                                                       
		None.

EFFECTS:                                                                
		Destination structure gets a copy of the full structure.

ROUTINE RETURN VALUES:                                                  
		None.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL copy(source, destination, byte_count)
char      *source;
char      *destination;
FAST ULONG byte_count;
{
     FAST ULONG index;

for (index = 0; index < byte_count; index++)
    destination[index] = source[index];
}

/******************************************************************************
TITLE:		FORMAT()
*******************************************************************************

DESCRIPTION:
		This function is responsible for formatting one track at a 
		time.  It copies a user format table into the local
		format table.  Then execute() is called to fill in
		the IOPB with the appropriate data, most importantly the
		cylinder and head values sent by the user.  If the operation
 		fails, the IOPB completion code and the offending sector
		is copied back into the user format table and returned.

REQUIRES:                                                               
		The user format table, including the cylinder and head values.
		The device number.

SUBROUTINE CALLS:                                                       
                init_iopb() - Initializes an IOPB.
		copyin() - Copies data from user data space in to kernel 
		    data space. (system)
		copyout() - Copies data from kernel data space out to user 
		    data space. (system)
		execute() - Executes an IOPB. 

EFFECTS:                                                                
		Formats one track.

ROUTINE RETURN VALUES:                                                  
		IOCTL_OK (0) if successfull, or
		IOCTL_FAILED (-1) if unsuccessfull.
       		If the format was unsuccessfull, the IOPB completion code and
		the failing sector is copied into the user format table.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL format(ioctl_table, devnum)
IOCTL_TABLE *ioctl_table;
int          devnum;
{
    char            status, *user_data = (char *)ioctl_table->ptr_1;
    BYTE            flag = NO;
    USHORT          unit;
    IOPBPTR         iopbptr;
    FORMAT_TABLE    format_table;

    unit = DRIVE(devnum);          /* Extract the unit number and then */
    iopbptr = riopb[unit];         /* get the associated raw IOPB.     */
    init_iopb(iopbptr, unit);      /* Initialize the associated raw IOPB. */

    status = copyin( user_data,                 /* Copy in the format table. */
                     (char *)&format_table,
                     sizeof(FORMAT_TABLE) );

    if ( status == -1 )
        return(IOCTL_FAILED);

    /* Fill in the applicable IOPB bytes. */

    iopbptr->byte[COUNTL] = ONE_TRACK;
    iopbptr->byte[CYLH] = (UCHAR)(format_table.cylinder >> 8);
    iopbptr->byte[CYLL] = (UCHAR)format_table.cylinder;
    iopbptr->byte[HEAD] = (UCHAR)format_table.head;

    /* Perform the actual FORMAT command. */

    if (execute(BY_INTERRUPT, XYFORMAT, devnum) IS_A FAILURE)
        {
        flag = YES;
        format_table.f_status = iopbptr->byte[COMPCODE];
        format_table.sector = iopbptr->byte[SECTOR];
        }

    status = copyout((char *)&format_table,
                              user_data,
                              sizeof(FORMAT_TABLE));

    if ( (status == -1) || (flag == YES) )
        return( IOCTL_FAILED );
    else
        return( IOCTL_OK );
}

/******************************************************************************
TITLE:		GET_TABLE()
*******************************************************************************

DESCRIPTION:
		This function is responsible for copying the contents of the
		driver's controller, unit, or partition information table
		out to a data area sent by the user.
                                                                            
REQUIRES:                                                               
		The driver's controller, unit or partition information table.
		A pointer to the user's area to store a copy of the table.
		A device number.

SUBROUTINE CALLS:                                                       
		copyout() - Copies from kernel space out to user space.

EFFECTS:                                                                
		Fills user's data area with a copy of the the appropriate 
		information table.

ROUTINE RETURN VALUES:                                                  
		The return value of copyout(): 
		    0 if successfull, or
		   -1 if unsuccessfull

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL get_table(cmd, ioctl_table, devnum)
int             cmd;
IOCTL_TABLE    *ioctl_table;
int             devnum;
{
    char              status, *user_data = (char *)ioctl_table->ptr_1;
    USHORT            unit;
    UNITINFO         *uinfo;

    unit = DRIVE(devnum);

    switch (cmd)   
        {                    
        case XYCTLRINFO:
                 
            status = copyout((char *)&ctlr_table, user_data, sizeof(CTLRINFO));
            
            if ( status == -1 )
                return( IOCTL_FAILED );

            break;

        case XYUNITINFO:

            status = copyout((char *)&unit_table[unit],
                                      user_data, 
                                      sizeof(UNITINFO));
            
            if ( status == -1 )
                return( IOCTL_FAILED );

            break;

        case XYPARTINFO:
            
            status = copyout((char *)&(part_table[(unit * PCNT) + FIRST_PARTITION]),
                                       user_data,
                                       PCNT * sizeof(PARTINFO));

            if ( status == -1 )
                return( IOCTL_FAILED );

            break;
    }

return( IOCTL_OK );
}

/******************************************************************************
TITLE:		HEADER_DATA()	
*******************************************************************************

DESCRIPTION:
      		This function is responsible for reading/writing track headers
		or reading/writing track data.  A work buffer is allocated for
                the controller to read/write the header/data information. The 
                modified work buffer address, the cylinder, and the head values 
		are stuffed into the IOPB.  If the command is a read/write track
		data command, the sector number and sector count are also put
 		into the IOPB.  If the command is write track header/data 
 		command, the header/data information is copied from the user 
		data area into the work buffer, and the command executed.  If 
		the command is a read track header/data command, the
		command is executed, and the work buffer data is copied into
		the user data area.

REQUIRES:                                                               
		A command, the ioctl table data, and the device number.

SUBROUTINE CALLS:                                                       
                init_iopb() - Initializes an IOPB.
                put_addr() - Puts an address into an IOPB.
                execute() - Executes an IOPB.
		copyin() - Copies data from user data space in to kernel 
		    data space. (system)
		copyout() - Copies data from kernel data space out to user 
		    data space. (system)
                rmalloc() - Allocates system memory. (system)
                rmfree() - De-allocates system memory. (system)

EFFECTS:                                                                
		The write track data commands over-write whatever
		was previously on the disk.

ROUTINE RETURN VALUES:                                                  
 		IOCTL_OK if successfull, or
		IOCTL_FAILED if the command or copyin/copyout fail.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL header_data(cmd, ioctl_table, devnum)
int               cmd;
IOCTL_TABLE      *ioctl_table;
int               devnum;
{
    USHORT        unit;
    IOPBPTR       iopbptr;
    UNITINFO     *uinfo;
    ULONG         address;

   unit = DRIVE(devnum);         /* Extract the unit number.      */
   iopbptr = riopb[unit];        /* Get the associated raw IOPB.  */
   uinfo = &unit_table[unit];    /* Point at the unit info-table. */

   init_iopb(iopbptr, unit);     /* Initialize the raw IOPB. */

    /* Allocate an appropriately-sized buffer. */

    address = (ULONG)work_buffer;     
    put_addr(iopbptr, address, DATA_ADDR);
    iopbptr->byte[DATA_MOD] = STD_SUPER_DATA;

    /* Stuff the data common to all commands into the IOPB. */

    iopbptr->byte[CYLH] = (UCHAR)(ioctl_table->var_1 >> 8);
    iopbptr->byte[CYLL] = (UCHAR)ioctl_table->var_1;
    iopbptr->byte[HEAD] = (UCHAR)ioctl_table->var_2;

    switch (cmd)
        {
        case XYREAD:     
                         iopbptr->byte[SECTOR] = (UCHAR)ioctl_table->var_3;
                         iopbptr->byte[COUNTL] = 1;

                         if ((execute(BY_INTERRUPT, XYREAD, devnum)
                             IS_A FAILURE) ||
                             (copyout(work_buffer,
                                     (char *)ioctl_table->ptr_1,
                                     uinfo->sect_size)
                             CAUSES_A BUS_ERROR))
		         {
			     ioctl_table->status = iopbptr->byte[COMPCODE];
                             return(IOCTL_FAILED);
			 }

                         break;

        case XYWRITE:   
                         iopbptr->byte[SECTOR] = (UCHAR)ioctl_table->var_3;
                         iopbptr->byte[COUNTL] = 1;

                         if ((copyin((char *)ioctl_table->ptr_1,
                                     work_buffer, 
                                     uinfo->sect_size)
                             CAUSES_A BUS_ERROR) ||
                             (execute(BY_INTERRUPT, XYWRITE, devnum)
                             IS_A FAILURE))
		         {
			     ioctl_table->status = iopbptr->byte[COMPCODE];
                             return(IOCTL_FAILED);
		         }

                         break;

        case XYRDDEFECTS: 
        case XYRDTRKHDR: if ((execute(BY_INTERRUPT, cmd, devnum)
                             IS_A FAILURE) ||
                             (copyout((caddr_t)work_buffer,
                                     (caddr_t)ioctl_table->ptr_1,
                                     cmd == XYRDTRKHDR ?
                                     (uinfo->sectors + uinfo->trk_spares) * 4 :
                                      DEFECT_MAP_SIZE )
                             CAUSES_A BUS_ERROR))
                         {
                             return(IOCTL_FAILED);
                         }

                         break;

        case XYWRTRKHDR: if ((copyin((caddr_t)ioctl_table->ptr_1,
                                    (caddr_t)work_buffer,
                                    (uinfo->sectors + uinfo->trk_spares) * 4)
                             CAUSES_A BUS_ERROR) ||
                             (execute(BY_INTERRUPT, XYWRTRKHDR, devnum)
                             IS_A FAILURE))
                         {
                             return(IOCTL_FAILED);
                         }

                         break;

    }/* switch */

return(IOCTL_OK); 
}

/******************************************************************************
TITLE:		READ_PARMS()
*******************************************************************************

DESCRIPTION:
		This function is resposible for reading controller or
		or drive parameters into a table supplied by the user.  
		An appropriate IOPB is sent to the controller to get
		some of the information, and the rest is supplied from
		the driver's internal information tables.  The driver's 
                internal tables will NOT be updated with any data read
		from the controller.
                                                                            
REQUIRES:                                                               
		A pointer to the user's data area.
		A device number.
                The controller information tables.

SUBROUTINE CALLS:                                                       
                init_iopb() - Initializes an IOPB.
                copy() - Copies one data structure to another.
		copyout() - Copies from kernel space to user space. (system)
		execute() - Executes an IOPB.

EFFECTS:                                                                
		In the case of controller or drive parameters, an IOPB 
		reflecting the user's data is sent to the controller.

ROUTINE RETURN VALUES:                                                  
		IOCTL_OK (0) if successfull, or
		IOCTL_FAILED (-1) if unsuccessfull, or
		Return status from copyout() (also 0 or -1).

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL read_parms (cmd, ioctl_table, devnum)
int               cmd;
IOCTL_TABLE      *ioctl_table;
int               devnum;
{
    char          status, *user_data = (char *)ioctl_table->ptr_1;
    USHORT        unit, i, j;
    IOPBPTR       iopbptr;
    CTLRINFO     *cinfo, temp_cinfo;
    UNITINFO     *uinfo, temp_uinfo;
    UCHAR        format_parms[FORMAT_PARMS];

    unit = DRIVE(devnum);          /* Extract the unit number.           */
    iopbptr = riopb[unit];         /* Point at the appropriate raw IOPB. */
    uinfo = &unit_table[unit];     /* Point at the driver's unit and     */
    cinfo = &ctlr_table;           /* controller information tables.     */

    init_iopb(iopbptr, unit);      /* Initialize the associated raw IOPB. */

    /* Send an IOPB requesting either controller or drive parameters.   */
    /* If successfull, copy the data from the IOPB to the user's table. */

    switch (cmd)
        {
        case XYRDCTLRPAR:

            if (execute(BY_INTERRUPT, XYRDCTLRPAR, devnum) IS_A FAILURE)
		return(IOCTL_FAILED);

            copy((char *)cinfo, (char *)&temp_cinfo, sizeof(CTLRINFO));

            temp_cinfo.param1 = iopbptr->byte[CTLRPAR1];
            temp_cinfo.param2 = iopbptr->byte[CTLRPAR2];
            temp_cinfo.param3 = iopbptr->byte[CTLRPAR3];
            temp_cinfo.throttle = iopbptr->byte[THROTTLE];
              
            status = copyout((char *)&temp_cinfo, user_data, sizeof(CTLRINFO));

            if ( status == -1 )
                return( IOCTL_FAILED );

            break;

        case XYRDDRVPAR:
        case XYDRVCONFIG:

            if (execute(BY_INTERRUPT, cmd, devnum) IS_A FAILURE)
		return(IOCTL_FAILED);

            copy((char *)uinfo, (char *)&temp_uinfo, sizeof(UNITINFO));        

            temp_uinfo.heads = iopbptr->byte[HEAD] + 1;
            temp_uinfo.cylinders = ((iopbptr->byte[CYLH] << 8) |
	                             iopbptr->byte[CYLL]) + 1;
            temp_uinfo.sectors = iopbptr->byte[SECTOR] + 1;

            if ( cmd == XYDRVCONFIG )
	        temp_uinfo.drive_parms = uinfo->drive_parms;
            else
                temp_uinfo.drive_parms = iopbptr->byte[DRV_PARM] & 
                                                                 INT_LEVEL_MASK;

            status = copyout((char *)&temp_uinfo, user_data, sizeof(UNITINFO));

            if ( status == -1 )
                return( IOCTL_FAILED );

            break;

        case XYOPTIONAL:

            iopbptr->byte[SUBFUNC] = (char)ioctl_table->var_1;
            iopbptr->byte[0x12] = (char)ioctl_table->var_2;
            iopbptr->byte[0x13] = (char)ioctl_table->var_3;

            if (execute(BY_INTERRUPT, XYOPTIONAL, devnum) IS_A FAILURE)
		return(IOCTL_FAILED);

            ioctl_table->long_var = (long)(iopbptr->byte[0x10] << 8) | 
                                          iopbptr->byte[0x11];

            break;

        case XYWHATAMI:

            if (execute(BY_INTERRUPT, XYRDDRVPAR, devnum) IS_A FAILURE)
		return(IOCTL_FAILED);

            /* Hi Nibble gets Partition Cnt, Lo Nibble gets Partition Number */
	    ioctl_table->status = (PCNT << 4) | PARTITION(devnum);

            ioctl_table->var_1 = Ctlr_Type;
            ioctl_table->var_2 = DISK;
            ioctl_table->var_3 = iopbptr->byte[ACT_SECTORS];
            ioctl_table->long_var = BSIZE;
	    break;

        case XYREVNO:

            if (execute(BY_INTERRUPT, XYRDCTLRPAR, devnum) IS_A FAILURE)
		return(IOCTL_FAILED);

            ioctl_table->var_1 = Major_Rev;
            ioctl_table->var_2 = Minor_Rev;

	    ioctl_table->var_3 = iopbptr->byte[SUBREV] |
		       ((UINT)iopbptr->byte[REVISION] << 8);

            ioctl_table->long_var = (iopbptr->byte[PART_HI] << 8) |
                        iopbptr->byte[PART_LO];
            break;

	case XYRDFORMPAR:

            if (execute(BY_INTERRUPT, XYRDFORMPAR, devnum) IS_A FAILURE)
		return(IOCTL_FAILED);

            if ((format_parms[F_PAR_STAT] = iopbptr->byte[COMPCODE]) 
                 IS F_PAR_OK)    
 	        {
                format_parms[0] = iopbptr->byte[INTERLEAVE] & INT_LEVEL_MASK;
                format_parms[1] = iopbptr->byte[FLD_1];
                format_parms[2] = iopbptr->byte[FLD_2];
                format_parms[3] = iopbptr->byte[FLD_3];
                format_parms[4] = iopbptr->byte[FLD_4];
                format_parms[5] = iopbptr->byte[FLD_5H];
                format_parms[6] = iopbptr->byte[FLD_5L];
                format_parms[7] = iopbptr->byte[FLD_5AH];
                format_parms[8] = iopbptr->byte[FLD_5AL];
                format_parms[9] = iopbptr->byte[FLD_6];
                format_parms[10] = iopbptr->byte[FLD_7];
                format_parms[11] = iopbptr->byte[FLD_12];
                }

            status = copyout((char *)&format_parms[0],
                                     user_data,
                                     FORMAT_PARMS);

            if ( status == -1 )
                return( IOCTL_FAILED );

    } /* End of switch. */

return( IOCTL_OK );
}

/******************************************************************************
TITLE:		SET_PARMS()
*******************************************************************************

DESCRIPTION:
		This function is resposible for setting a controller's 
		parameters with values supplied by the user.  It will
		will send an IOPB to the controller if necessary, and update
		either the driver's controller, unit, or unit partition
		table, depending upon the command sent.
                                                                            
REQUIRES:                                                               
		The ioctl() comand.
		A pointer to the user's data area.
		The device number.            

SUBROUTINE CALLS:                                                       
                init_iopb() - Initializes an IOPB.
                copy() - Copies one data structure to another.
		copyin() - Copies from user space to kernel space. (system)
		execute() - Executes an IOPB.

EFFECTS:                                                                
		In the case of controller or drive parameters, an IOPB 
		reflecting the user's data is sent to the controller.
		Partition parameters are just stuffed into the table.

ROUTINE RETURN VALUES:                                                  
		IOCTL_OK (0) if successfull, or
		IOCTL_FAILED (-1) if unsuccessfull, or
		Return status from copyout() (also 0 or -1).

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL set_parms (cmd, ioctl_table, devnum)
int              cmd;
IOCTL_TABLE     *ioctl_table;
int              devnum;
{
    char       status, *user_data = (char *)ioctl_table->ptr_1;
    USHORT     unit, i, j;
    CTLRINFO  *cinfo, ctlr_backup;
    UNITINFO  *uinfo, unit_backup;
    PARTINFO  *pinfo, part_backup[PCNT];
    IOPBPTR    iopbptr;
    UCHAR      format_parms[FORMAT_PARMS];

    unit = DRIVE(devnum);       /* Set up the local variables and pointers. */
    uinfo = &unit_table[unit];  
    cinfo = &ctlr_table;                       
    pinfo = (PARTINFO *)&(part_table[(unit * PCNT) + FIRST_PARTITION]);

    iopbptr = riopb[unit];
    init_iopb(iopbptr, unit);   /* Initialize the associated raw IOPB. */
                    
                              /* When setting controller or unit parameters, */
    switch (cmd)              /* stash the current table first.  If either   */
        {                     /* copyin() or  execute() fail, restore it and */
                              /* and return IOCTL_FAILED to ioctl().         */
        case XYSETCTLRPAR:

            copy((char *)cinfo, (char *)&ctlr_backup, sizeof(CTLRINFO));

            status = copyin( user_data, (char *)cinfo, sizeof(CTLRINFO) );

            if ( status == -1 )        /* copyin Failed */
                return( IOCTL_FAILED );

            if ( execute(BY_INTERRUPT, XYSETCTLRPAR, devnum) IS_A FAILURE )
                {
                copy((char *)&ctlr_backup, (char *)cinfo, sizeof(CTLRINFO));
                return(IOCTL_FAILED);
                }

            break;

        case XYSETDRVPAR:    

            copy((char *)uinfo, (char *)&unit_backup, sizeof(UNITINFO));

            status = copyin( user_data, (char *)uinfo, sizeof(UNITINFO) );

            if ( status == -1 )        /* copyin Failed */
                return( IOCTL_FAILED );

            if ( execute( BY_INTERRUPT, XYSETDRVPAR, devnum ) IS_A FAILURE)
                {
                copy((char *)&unit_backup, (char *)uinfo, sizeof(UNITINFO));
                return(IOCTL_FAILED);
                }

            break;

        case XYSETPARTS:

            copy((char *)pinfo, (char *)part_backup,
                  PCNT * sizeof(PARTINFO));

            status = copyin(user_data, (char *)pinfo, PCNT * sizeof(PARTINFO));

            if ( status == -1 )        /* copyin Failed */
                {
                copy((char *)part_backup, (char *)pinfo,
                       PCNT * sizeof(PARTINFO));

                return( IOCTL_FAILED );
                }

            break;

	case XYSETFORMPAR:

           status = copyin(user_data,(char *)format_parms,FORMAT_PARMS);

            if ( status == -1 )        /* copyin Failed */
                return( IOCTL_FAILED );

            iopbptr->byte[INTERLEAVE] |= format_parms[0] & INT_LEVEL_MASK;
            iopbptr->byte[FLD_1] = format_parms[1];
            iopbptr->byte[FLD_2] = format_parms[2];
            iopbptr->byte[FLD_3] = format_parms[3];
            iopbptr->byte[FLD_4] = format_parms[4];
            iopbptr->byte[FLD_5H] = format_parms[5];
            iopbptr->byte[FLD_5L] = format_parms[6];
            iopbptr->byte[FLD_5AH] = format_parms[7];
            iopbptr->byte[FLD_5AL] = format_parms[8];
            iopbptr->byte[FLD_6] = format_parms[9];
            iopbptr->byte[FLD_7] = format_parms[10];
            iopbptr->byte[FLD_12] = format_parms[11];

            execute(BY_INTERRUPT, XYSETFORMPAR, devnum);

            format_parms[F_PAR_STAT] = iopbptr->byte[COMPCODE];

            status = copyout((char *)format_parms,
                                     user_data,
                                     FORMAT_PARMS);

            if ( status == -1 )        /* copyout Failed */
                return( IOCTL_FAILED );

    } /* End of switch. */

return(IOCTL_OK);
}


/******************************************************************************
FILE = %W%	%G%	Inc.
*******************************************************************************
TITLE:		DUMP_IOPB()
*******************************************************************************

DESCRIPTION:
		This function displays a range of IOPB bytes.  It simply
		leads into disp_iopb().

REQUIRES:                                                               
		A pointer to the IOPB to be displayed.
		The first and last bytes in the IOPB to be displayed.

SUBROUTINE CALLS:                                                       
		dri_printf() - Prints to the console. (system)

EFFECTS:                                                                

ROUTINE RETURN VALUES:                                                  

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL dump_iopb (iopbptr, first, last)
IOPBPTR          iopbptr;
FAST USHORT      first, last;
{
     FAST USHORT offset;

if (last >= ISIZE)
    dri_printf("WARNING - Attempting To Display Non-informational IOPB Bytes\n");

dri_printf("\nIOPB @ 0x%x Bytes 0x%x thru 0x%x\n", iopbptr, first, last );

for (offset = first; offset <= last; ++offset)
    dri_printf("BYTE[0x%x]>>> 0x%x\n", offset, iopbptr->byte[offset] );
}

/******************************************************************************
FILE = %W%	%G%	Inc.
*******************************************************************************
TITLE:		EXECUTE()
*******************************************************************************

DESCRIPTION:
		This function will build and execute an IOPB for any  
		command.  It will do so in a polled or interrupt manner.
                                                                            
REQUIRES:                                                               
		A manner of execution. 
                A command.
                A unit or dev_t, both of which are defined as USHORT.
		The unit information and controller information tables.

SUBROUTINE CALLS:                                                       
		init_iopb() - Initializes a generic IOPB.
		poll() - Handles interaction with controller for actual
		    execution of an IOPB in a polled fashion.
 		make_checksum() - Generates an IOPB checksum.

EFFECTS:                                                                
		Inserts the appropriate data into a raw IOPB.
		Interacts with the controller via execute().
		A controller reset utilizes the CSR rather than sending an IOPB.

ROUTINE RETURN VALUES:                                                  
		SUCCESS upon successfull execution of the command.
		FAILURE upon unsuccessfull execution of the command.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL execute( manner, command, device )
FAST int       manner;
FAST int       command;
FAST USHORT    device;   /* This is either a unit number or a dev_t number. */
{
    UNITINFO  *uinfo;
    CTLRINFO  *cinfo;
    IOPBPTR    iopbptr;
    BUFPTR     bufptr;
    USHORT     unit;

unit = (manner IS POLLED) ? device : DRIVE(device);  /* Set unit accordingly. */

uinfo = &unit_table[unit];            /* Point at the unit info-table.       */
cinfo = &ctlr_table;                  /* Point at the controller info-table. */
iopbptr = riopb[unit];                /* Point at the associated raw IOPB.   */

if (manner IS POLLED)
{
    if(command == XYSETFORMPAR)
	prep_iopb(iopbptr, unit);	  /* dont clear the IOPB bytes */
    else
    	init_iopb(iopbptr, unit);         /* Initialize the IOPB, and */

    iopbptr->byte[LEVEL] = DISABLED;   /* then disable interrupts. */
}

if (manner IS BY_INTERRUPT)
{
    bufptr = &rbuf[unit];               /* Point at the associated raw buf.  */
    while (bufptr->b_flags & B_BUSY)
    {                                    /* If the raw buf is B_BUSY, set the */
        bufptr->b_flags |= B_WANTED;     /* B_WANTED flag and wait patiently. */
        sleep((caddr_t)bufptr, PRIBIO + 1);              
    }
    bufptr->b_flags = B_RAWBUF | B_BUSY; /* Set B_RAWBUF and B_BUSY ONLY!!!   */
    bufptr->b_dev = device;              /* Copy in the dev_t device number.  */
    bufptr->b_bcount = (long)iopbptr;    /* Use b_bcount as an IOPB pointer.  */
    bufptr->b_error = CLEAR;             /* Clear the b_error flag.           */
    iopbptr->buffer = bufptr;            /* Have the IOPB point at this buf.  */
}

switch (command)
{
    /* Set up the IOPB by inserting the command, subfunction, and */
    /* any necessary parameters that haven't already been stuffed */
    /* into their appropriate fields.                             */

    case XYSETCTLRPAR:  iopbptr->byte[COMMAND] |= Write_Par;
                        iopbptr->byte[SUBFUNC] |= W_Controller;
                        iopbptr->byte[CTLRPAR1] = cinfo->param1;
                        iopbptr->byte[CTLRPAR2] = cinfo->param2;
                        iopbptr->byte[CTLRPAR3] = cinfo->param3;
                        iopbptr->byte[THROTTLE] = cinfo->throttle;
                        iopbptr->byte[0x13] = Ctype;
                        break;

    case XYSETDRVPAR:   iopbptr->byte[COMMAND] |= Write_Par;
                        iopbptr->byte[SUBFUNC] |= W_Drive;
                        iopbptr->byte[DRV_PARM] |= uinfo->drive_parms;
                        iopbptr->byte[HEAD] = uinfo->heads - 1;
                        iopbptr->byte[SECTOR] = uinfo->sectors - 1;
                        iopbptr->byte[MAXSECLH] = uinfo->sectors - 1;
                        iopbptr->byte[ACT_SECTORS] = uinfo->sectors;
                        iopbptr->byte[CYLL] = uinfo->cylinders - 1;
                        iopbptr->byte[CYLH] = (uinfo->cylinders - 1) >> 8;
                        break;

    case XYRDCTLRPAR:   iopbptr->byte[COMMAND] = Read_Par;
                        iopbptr->byte[SUBFUNC] = R_Controller;
                        break;

    case XYRDDRVPAR:    iopbptr->byte[COMMAND] = Read_Par;
                        iopbptr->byte[SUBFUNC] = R_Drive;
                        break;

    case XYDRVCONFIG:   iopbptr->byte[COMMAND] = Read_Par;
                        iopbptr->byte[SUBFUNC] = Drv_Conf;
                        break;

    case XYRDDEFECTS:   iopbptr->byte[COMMAND] = Ext_Read;
                        iopbptr->byte[SUBFUNC] = R_Def;
                        break;

    case XYFORMAT:      iopbptr->byte[COMMAND] = Ext_Write;
                        iopbptr->byte[SUBFUNC] = W_Format;
                        break;

    case XYRDTRKHDR:    iopbptr->byte[COMMAND] = Ext_Read;
                        iopbptr->byte[SUBFUNC] = R_Trk_Hdr;
                        break;

    case XYWRTRKHDR:    iopbptr->byte[COMMAND] = Ext_Write;
                        iopbptr->byte[SUBFUNC] = W_Trk_Hdr;
                        break;

    case XYDRVRESET:    iopbptr->byte[COMMAND] = Drive_Reset;
                        break;

    case XYOPTIONAL:    iopbptr->byte[COMMAND] = Opt_Cmd;
                        break;

    case XYREAD:        iopbptr->byte[COMMAND] = Read;
 			break;

    case XYWRITE:       iopbptr->byte[COMMAND] = Write;
 			break;

    case XYRDFORMPAR:	iopbptr->byte[COMMAND] = Read_Par;
			iopbptr->byte[SUBFUNC] = R_Format;
 			break;

    case XYSETFORMPAR:	iopbptr->byte[COMMAND] = Write_Par;
			iopbptr->byte[SUBFUNC] = W_Format;
 			break;

} /* End of switch. */

make_checksum(iopbptr);     /* Generate a checksum to avoid any hangups. */

if (manner IS BY_INTERRUPT) 
{
    m323strategy(bufptr);       /* Execute the IOPB in an interrupt manner. */
    iowait(bufptr);           /* Wait here until the buf is completed.    */

    if (bufptr->b_error & ENXIO)
    {                                        /* strategy() has determined   */
                                             /* this buf to be incorrect. */
        iopbptr->byte[COMMAND] |= ERRS;      /* ERRS for the hook below.    */
    }

    bufptr->b_flags &= ~B_BUSY;   /* Clear B_BUSY for the next ioctl() call. */

    if (bufptr->b_flags & B_WANTED)   /* If the buf is B_WANTED, wake up the */
        wakeup((caddr_t)bufptr);      /* caller who's requesting it above.   */

    if (iopbptr->byte[COMMAND] & ERRS)      /* Check the ERRS bit for errors. */
        return(FAILURE);
    else
        return(SUCCESS);
}

return(poll(iopbptr));      /* Execute the IOPB in a polled manner.   */
                            /* Return the status returned by poll(). */

}

/******************************************************************************
TITLE:		POLL()
*******************************************************************************

DESCRIPTION:
		This function executes a discrete IOPB in a "polled" fashion.
		First it waits for the RBS and AIOP bits to clear.  It then 
		stuffs the address of the IOPB into the controllers address 
		registers, and sets AIO.  When RIO is set by the controller,
		the ERRS bit is checked for execution errors.  If OK, the
		function returns SUCCESS, otherwise FAILURE. 
                                                                            
REQUIRES:                                                               
		A pointer to the IOPB to be executed.

SUBROUTINE CALLS:                                                       
		stuff_addr() - Stuffs an address into the controller's
		    address registers.
		test_bit() - Tests a CSR register bit. (via TEST_(x) macro)
		set_bit() - Sets a CSR register bit. (via SET_(x) macro)
		clear_bit() - Clears a CSR register bit. (via CLEAR_(x) macro)

EFFECTS:                                                                
		This routine performs all of the machinations that normally
		would be done by a call to strategy(), except that the 
		interrupt handler is not invoked.

ROUTINE RETURN VALUES:                                                  
		SUCCESS if IOPB is executed normally, or
		FAILURE if RBS is set, RIO is not set in a reasonable amount
 		of time, or ERRS is set in the returned IOPB.

*******************************************************************************
EDIT HISTORY:
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL poll (iopbptr)
IOPBPTR     iopbptr;
{
    USHORT   timeout, flip_flop = 0;
    ULONG    cnt;

    if (TEST_(RBS) IS_NOT CLEAR)
    {
        /* If RBS is not clear, bail out. */
    
        dri_printf("M323:  RBS NOT CLEAR\n");
        return(FAILURE);
    }

    while (TEST_(AIOP) IS_NOT CLEAR )  /* Wait for AIOP to clear. */ 
    {
	if(TEST_(FERR) IS_NOT CLEAR)  
		cmn_err(CE_PANIC, "M323: controller reported fatal error\n");
    }
    
    stuff_addr(iopbptr);             /* Set up the address registers. */
    
    SET_(AIO);                        /* Start the controller by setting AIO. */ 
    for ( cnt = 0; TEST_(RIO) IS CLEAR; ++cnt)
        {
        if ( cnt >= LOOP_LIMIT )
            {
            dri_printf("M323:  RIO NEVER SET\n");
            CLEAR_(RBS);
            return(FAILURE);
            }
        else
            if ( (cnt % 500) IS 499 )
                dri_printf("M323: WAITING ON RIO...CSR=> 0x%x  \r", read_reg(CSR) );
        }
    
    CLEAR_(RIO);     /* Clear the RIO and RBS bits. */
    CLEAR_(RBS);                                 
    
    if (iopbptr->byte[COMMAND] & ERRS)   /* Check the ERRS bit for success. */
        return(FAILURE);
    else
        return(SUCCESS);
}

/******************************************************************************
FILE = %W%	%G%	Inc.
*******************************************************************************
TITLE:		GO()
*******************************************************************************

DESCRIPTION:
		This function, called by strategy() and interrupt(), stuffs the
		address of the top IOPB in the READY IOPB queue into the 
		controller's address registers and then sets AIO in order
		to set the controller in motion.  It must wait for RBS and
		AIOP to clear before setting AIO.
                                                                            
REQUIRES:                                                               
		At least one IOPB to be on the READY IOPB queue.

SUBROUTINE CALLS:                                                       
		stuff_addr() - Puts an IOPB address into the controller's
		    registers for execution.
		test_bit() - Tests a CSR register bit. (via TEST_() macro)
		set_bit() - Sets a CSR register bit. (via SET_() macro) 
		flush_queue() - Flushes a queue.

EFFECTS:                                                                
		Sets the controller in motion.

ROUTINE RETURN VALUES:                                                  
		None.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL go()
{

    while (TEST_(RBS) IS_NOT CLEAR)   /* Wait for RBS and AIOP to clear */ 
    {
	if(TEST_(FERR) IS_NOT CLEAR)  
		cmn_err(CE_PANIC, "M323: controller reported fatal error\n");
    }
      
                                           /* before accessing the AIO bit.  */
    while (TEST_(AIOP) IS_NOT CLEAR)
    {
	if(TEST_(FERR) IS_NOT CLEAR)  
		cmn_err(CE_PANIC, "M323: controller reported fatal error\n");
    }
                              /* Load the controller's address registers with */
    stuff_addr(READY_QUEUE); /* the first IOPB in the READY IOPB queue.      */
    SET_(AIO);             /* Activate the controller by setting the AIO bit. */

    flush_queue(Queue.ready);                /* Flush the READY IOPB queue. */

    CLEAR_(RBS);                  /* We're done manipulating the controller's */
                                  /* registers, so clear the RBS bit.         */
}

/******************************************************************************
FILE = %W%	%G%	Inc.
*******************************************************************************
TITLE:		INIT()
*******************************************************************************

DESCRIPTION:
                                                                            
           Check if a controller exists at the configured address

REQUIRES:                                                               

SUBROUTINE CALLS: attach() - Set up Driver for any Disk Drives that should be 
                             out there.
                  clear_bit() - Set a Bit in a Register to 0.
                  init_queues() - Get the BUF/IOPB Queues Ready for Use.

RETURN VALUE: None

ROUTINE RETURN VALUES:                                                  

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

m323init()
{
    CTLRINFO       *cinfo = &ctlr_table;
    UNITINFO       *uinfo;
    USHORT          status;
    USHORT          unit, controller, max_ctlrs;
    UCHAR	    ctlralive;
    register struct iobuf *dp;

max_ctlrs = m323_cnt / UCNT;

if ( Boot_Flag )
    init_queues(YES);                           /* Initialize IOPB/BUF Queue */

for ( controller = 0; controller < max_ctlrs; ++controller )
    {
    ctlralive = FALSE;
    cinfo->reg_address = (REGISTERS *)m323_addr[controller];   

          /* SEE IF THE CONTROLLER IS OUT THERE */

#ifdef DEBUG
    dri_printf("MVME323 Disk Controller at 0x%x",&cinfo->reg_address->fill1);
#endif
	/* Probe by reading the status register. This will cause the 
	   controller to set RBS (register busy semaphore) in the status
	   register for all subsequent reads until the RBS bit is cleared
	   by setting the "Clear RBS" bit in the command register (the 
	   code is the "clear_bit(RBS)" statement performed soon after the
	   bprobe.
	*/
    if ( (status = bprobe( &cinfo->reg_address->csr, -1 )) NOT= 0 )
        {
#ifdef DEBUG
        dri_printf("...NOT FOUND\n");
#endif
        cinfo->c_status = ABSENT;
        }
    else
        {
#ifdef DEBUG
        dri_printf("...FOUND\n");
#endif
        cinfo->c_status = PRESENT;

	 /* clear the "register busy semaphore" bit which was set by the 
	    controller after bprobe read the status register. */
        clear_bit( RBS );

        for ( unit = 0; unit < UCNT; ++unit )
            {
            uinfo = &unit_table[(controller * M323DRV) + unit];

            uinfo->int_level = m323_ilev[controller];
            uinfo->vector = m323_ivec[controller] / 4;
            uinfo->reg_address = cinfo->reg_address;

	    dp = &m323utab[(controller * M323DRV) + unit];
	    dp->b_flags = B_CONF;    /* read config off disk at firstopen. */

	   /* set up for error logging */
	    dp->io_addr = (paddr_t)&m323eregs[controller];
	    dp->io_nreg = M323EREG;
	    dp->io_stp = (struct iostat *)&m323stat[(controller * M323DRV) + unit];
	   
            if(attach( unit ))        /* Try to Attach this Controller's Units */
		ctlralive = TRUE;	
            }
        }
	if(ctlralive == TRUE)
		setformparm(controller);
    }
}

/******************************************************************************
FILE = %W%	%G%	Inc.
*******************************************************************************
TITLE:		INTR()
*******************************************************************************

DESCRIPTION:
		This is the interrupt service routine for this controller.  It
		first makes sure it is OK to look at the controller's registers
		by waiting for RBS to clear.  Then it verifies that the 
		interrupt is meant for this unit by ensuring that RIO was set 
		by the controller.  If it is a legitimate interrupt, it reads 
		the IOPB address from the controller's address registers, and 
		then clears RIO and RBS.  Then the Main Bus DVMA memory map 
		resources are released.  The buf structure associated with the
		IOPB is updated by update_iopb(), and the IOPB is returned to 
		the FREE IOPB queue.  In the same manner as in strategy(), 
		any available buf structures are linked with any available 
		IOPBs and sent to the controller for immediate execution.
                                                                            
REQUIRES:                                                               
		The unit number that has interrupted.

SUBROUTINE CALLS:                                                       
		valid_iopb() - Checks validity of an IOPB address.
		dri_printf() - Prints a string to the console. (system)
		test_bit() - Tests a CSR register bit. (via TEST_() macro)
		clear_bit() - Clears a CSR register bit. (via CLEAR_() macro)
		read_addr() - Reads IOPB address from controller registers.
		update_buf() - Updates buf structure before returning it
		    to the operating system.
		enq_iopb() - Enqueues an IOPB onto specified queue.
		buf_to_iopb() - Attempts to make IOPBs out of bufs.
		go() - Sets the controller in motion with READY IOPB queue.

EFFECTS:                                                                
		This routine grabs control of the controllers registers for 
		the time necessary to read the address registers.
		The buf structure is updated for system use.
		Any waiting buf structures are linked with any available
		IOPBs, and sent to the controller for execution.

ROUTINE RETURN VALUES:                                                  
		SERVICED (1) if interrupt was serviced.
		NOT_SERVICED (0) if interrupt was not for this unit.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

m323intr(unit)
int    unit;
{
    IOPBPTR       done_iopb, read_addr();
    BOOL          valid_iopb();

    /* First wait for the register semaphore (RBS) is clear. */
    /* Then confirm that the interrupt was meant for this    */
    /* unit by checking whether the controller has set RIO.  */
    while (TEST_(RBS) IS_NOT CLEAR )
    {
	if(TEST_(FERR) IS_NOT CLEAR)  
		cmn_err(CE_PANIC, "M323: controller reported fatal error\n");
    }

    if (TEST_(RIO) IS SET)
    {
        /* If this interrupt is ours, then get the address of the  */
        /* completed IOPB from the controller's address registers. */

        done_iopb = read_addr();
        CLEAR_(RIO);   /* Clear the RIO and RBS bits in the CSR. */
        CLEAR_(RBS);

        if (valid_iopb(done_iopb))
        {
	    /* Update the buf structure before returning it. */
 	    /* Then queue the IOPB back onto the FREE QUEUE. */

	    /* If interrupt at end of chain is to be supported then code
	       needs to be added here to return all chained IOPBs to the
	       free queue. */
            update_buf(done_iopb);     
	    done_iopb->flags &= ~IOPBACTIVE;
	    /* If the controller does not have any IOPBs queued, then
	       zero the count of IOPBs sent to the controller, since it was
	       last flushed forceably. */ 
	    if(--m323qcnt <= 0)	/* decrement count of uncompleted IOPBs */
	    {
		    m323qcnt = 0;
		    m323rcnt = 0;
		    m323cflush = FALSE;
	    }
	    else
	    {
		    /* Count the number of IOPBs completed since the last
		       time the controllers queue was empty. Determine
		       when it is necessary to withhold IOPBs from the 
		       controller to allow the controller to complete all
		       the IOPBs in its queue. */
	    	    if(++m323rcnt >= FLUSHLIM)
		    {
			m323cflush = TRUE; /*allow controller to empty its 
					     queue */
		    }
	    }
            enq_iopb(Queue.free, done_iopb); 

        }
	else
            cmn_err(CE_PANIC,"MVME323 returned bogus IOPB address=> 0x%x\n",
                    done_iopb);

        /* If any more buf structures are available, attempt to make */
        /* IOPB's out of them, if possible.  If it IS possible, then */
        /* ship 'em out to the controller here, by calling go().     */

        if (buf_to_iopb())  
            go();               

        return(SERVICED);  
    }
    else                    
    {
        /* Even though the interrupt was NOT for this device, */
        /* we still have to clear RBS before returning.       */ 

        CLEAR_(RBS);
        return(NOT_SERVICED);     
    }
}

/******************************************************************************
FILE = %W%	%G%	Inc.
*******************************************************************************
TITLE:		IOCTL()
*******************************************************************************

DESCRIPTION:
		This function is the Special Interface Function for this
		device, performing routines such as formatting, en/disabling
		switches, setting parameters, displaying internal information,
		resetting the controller and drives, and reading or writing
		headers or data.s.
                                                                            
REQUIRES:                                                               
		A device number.
		An encoded command between the caller and this routine.
		An ioctl table containing any additional necessary data or
		pointers.

SUBROUTINE CALLS:                                                       
                rmalloc() - Allocates system memory. (system)
		get_table() - Gets requested data and copies it into the
		    user's ioctl table.
                read_parms() - Reads parameters into user table.
		set_parms() - Sets parameters sent by the user. 
 		format() - Formats one track.
 		header_data() - Reads/writes track header/data.
 		execute() - Executes an IOPB.
		dri_printf() - Prints to console. (system)

EFFECTS:                                                                
		Too numerous to mention here.

ROUTINE RETURN VALUES:                                                  

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

m323ioctl(dev, cmd, arg, mode)
int     dev;
int     cmd;
char   *arg;
int     mode;
{
    IOCTL_TABLE   *ioctl_table, local_table;
    short         status;
    int           unit, header_data();

    COPY_IN( arg, local_table )

    ioctl_table = &local_table;

    switch (cmd)
    {
        case XYWHATAMI:
        case XYREVNO:
        case XYRDCTLRPAR:
        case XYRDDRVPAR:
 	case XYRDFORMPAR:
 	case XYDRVCONFIG:
 	case XYOPTIONAL:

            if ( (status = read_parms(cmd, ioctl_table, dev) ) NOT= IOCTL_OK )
                u.u_error = EIO;
            else
                u.u_error = IOCTL_OK;
            break;

        case XYCTLRINFO:
        case XYUNITINFO:
        case XYPARTINFO:

            if ( (status = get_table(cmd, ioctl_table, dev) ) NOT= IOCTL_OK )
                u.u_error = EIO;
            else
                u.u_error = IOCTL_OK;
            break;

        case XYSETCTLRPAR:
        case XYSETDRVPAR: 
        case XYSETPARTS: 
        case XYSETFORMPAR:

            if ( (status = set_parms(cmd, ioctl_table, dev) ) NOT= IOCTL_OK )
                u.u_error = EIO;
            else
                u.u_error = IOCTL_OK;
            break;

        case XYFORMAT:       

            if ( (status = format( ioctl_table, dev ) ) NOT= IOCTL_OK )
                u.u_error = EIO;
            else
                u.u_error = IOCTL_OK;
            break;

        case XYREAD:
        case XYWRITE:        
        case XYRDTRKHDR:        
        case XYWRTRKHDR:        
        case XYRDDEFECTS:

            if ( (status = header_data( cmd, ioctl_table, dev) ) NOT= IOCTL_OK )
                u.u_error = EIO;
            else
                u.u_error = IOCTL_OK;
            break;

        case XYDRVRESET:

            unit = DRIVE(dev);
            init_iopb(riopb[unit], unit);

            if ( execute(BY_INTERRUPT, cmd, dev) IS_A FAILURE )
                u.u_error = EIO;
            else
                u.u_error = IOCTL_OK;
            break;

        case XYRESETCTLR: 
            
            write_reg(CSR, RESET); 
            u.u_error = IOCTL_OK;
            break;

        default: 

            u.u_error = EINVAL;

    } /* End of switch. */

COPY_OUT( arg, ioctl_table );

if ( u.u_error IS_NOT IOCTL_OK )
    return( IOCTL_FAILED );
else
    return( IOCTL_OK );
}

/******************************************************************************
FILE = %W%	%G%	Inc.
*******************************************************************************
TITLE:		GET_ADDR() 
*******************************************************************************

DESCRIPTION:
		This function builds a 32 bit address from either an IOPB's
		next IOPB bytes or the data address bytes.

REQUIRES:                                                               
		An IOPB pointer.
		Either a NEXT_IOPB, DATA_ADDR or LAST_IOPB flag.

SUBROUTINE CALLS:                                                       
		None.

EFFECTS:                                                                
		None.

ROUTINE RETURN VALUES:                                                  
		A 32 bit address derived from the IOPB.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL ULONG  get_addr (iopbptr, where)
IOPBPTR                iopbptr;
UCHAR                   where;
{
    ULONG  address;

    switch (where)
    {
        case NEXT_IOPB:   address = iopbptr->byte[NIOPB1] << 24;
                          address |= iopbptr->byte[NIOPB2] << 16;
                          address |= iopbptr->byte[NIOPB3] << 8;
                          address |= iopbptr->byte[NIOPB4];
		          break;

	case LAST_IOPB:
        case DATA_ADDR:   address = iopbptr->byte[DATA1] << 24;
                          address |= iopbptr->byte[DATA2] << 16;
                          address |= iopbptr->byte[DATA3] << 8;
                          address |= iopbptr->byte[DATA4];
			  break;
    }
    return(address);
}

/******************************************************************************
TITLE:		INIT_IOPB()
*******************************************************************************

DESCRIPTION:
		This function initializes a general purpose IOPB.
                                                                            
REQUIRES:                                                               
		Pointer to the IOPB.
		The unit number.
		The unit information table.

SUBROUTINE CALLS:                                                       
		None.

EFFECTS:                                                                
		Clears out remnants of old IOPBs, and inserts the unit number,
		interrupt level and interrupt vector values.

ROUTINE RETURN VALUES:                                                  
		None.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL init_iopb (iopbptr, unit)
IOPBPTR          iopbptr;
USHORT           unit;
{
    UNITINFO   *uinfo;
    FAST USHORT offset;

    uinfo = &unit_table[unit];

    /* First clear all of the IOPB bytes. */

    for ( offset = 0; offset < ISIZE; ++offset)  
    	iopbptr->byte[offset] = CLEAR;

    /* Then clear the address pointers. */
    iopbptr->buffer = NULL;

    /* Insert the unit number, interrupt level, and vector values. */

    iopbptr->byte[UNIT] |= (UCHAR)unit;
    iopbptr->byte[LEVEL] = (UCHAR)uinfo->int_level;
    iopbptr->byte[VECTOR] = (UCHAR)uinfo->vector;
}

/* Same as init_iopb except the IOPB bytes are not cleared. */
LOCAL prep_iopb (iopbptr, unit)
IOPBPTR          iopbptr;
USHORT           unit;
{
    UNITINFO   *uinfo;
    FAST USHORT offset;

    uinfo = &unit_table[unit];

    /* Then clear the address pointers. */
    iopbptr->buffer = NULL;

    /* Insert the unit number, interrupt level, and vector values. */

    iopbptr->byte[UNIT] |= (UCHAR)unit;
    iopbptr->byte[LEVEL] = (UCHAR)uinfo->int_level;
    iopbptr->byte[VECTOR] = (UCHAR)uinfo->vector;
}

/******************************************************************************

/******************************************************************************
TITLE:		MAKE_CHECKSUM()
*******************************************************************************

DESCRIPTION:
		This function generates a checksum of an IOPB and stuffs
		the value into the checksum bytes of the IOPB. 
REQUIRES:                                                               
		An IOPB pointer.

SUBROUTINE CALLS:                                                       
		None.

EFFECTS:                                                                
		Updates the checksum bytes of the IOPB.

ROUTINE RETURN VALUES:                                                  
		None.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL make_checksum(iopbptr)
IOPBPTR             iopbptr;
{
    FAST ULONG  checksum;
    FAST USHORT index;

    for(index = 0, checksum = 0; index < CHECKH; index++)
	checksum += iopbptr->byte[index];

    iopbptr->byte[CHECKH] = (UCHAR)(checksum >> 8);
    iopbptr->byte[CHECKL] = (UCHAR)checksum;
}

/******************************************************************************
TITLE:		MAKE_IOPB()
*******************************************************************************

DESCRIPTION:
		This function builds an executable IOPB from a buf structure
		and the appropriate unit information table entries.

REQUIRES:                                                               
		Pointers to an IOPB and a buf structure.

SUBROUTINE CALLS:                                                       
		init_iopb() - Initializes a generic IOPB.
		copy() - Copies data from one location to another.
		convert_blkno() - Converts a logical block number from the 
		    operating system into a cylinder, head and sector numbers.

EFFECTS:                                                                
		The IOPB returned from this function is ready to be queued onto
		the ready IOPB queue.  The data address is converted to a form
		useable by the controller.

ROUTINE RETURN VALUES:                                                  
		None.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL make_iopb (iopbptr, bufptr)
IOPBPTR          iopbptr;
BUFPTR           bufptr;
{
    UNITINFO      *uinfo;
    PARTINFO      *pinfo;
    ULONG          data_addr;
    USHORT         part, unit, cylinder, head, sector, count;

    unit = DRIVE(bufptr->b_dev);     /* Set up local variables and pointers. */
    part = PARTITION(bufptr->b_dev);     
    uinfo = &unit_table[unit];
    pinfo = (PARTINFO *)&(part_table[(unit * PCNT) + part]);

    if RAWBUF(bufptr)  /* If it's a raw buf, the IOPB has already been made. */
    {
        copy(bufptr->b_bcount, iopbptr, sizeof(IOPB));
        return;
    }


    /* We have to create the IOPB from scratch.  After init_iopb()  */
    /* clears all of the IOPB bytes, it stuffs the unit number, the */
    /* interrupt level, and interrupt vector.                       */

    init_iopb(iopbptr, unit);

    if (bufptr->b_flags & B_READ)      
        iopbptr->byte[COMMAND] = Read;
    else                                   /* Insert the command. */
        iopbptr->byte[COMMAND] = Write;

    /* Utilizing the arguments sent to it, convert_blkno() will  */
    /* compute and insert the head, sector, and cylinder numbers */
    /* into the HEAD, SECTOR, CYLL and CYLH fields of the IOPB.  */

    if(bufptr->b_flags & B_ABS)   /* do not add offset to b_blkno */
    	convert_blkno(bufptr->b_blkno, unit, iopbptr);
    else
    	convert_blkno(bufptr->b_blkno + pinfo->first, unit, iopbptr);

    /* Convert the byte count into a sector count. */

    iopbptr->byte[COUNTL] = (UCHAR)(bufptr->b_bcount/BSIZE);
    iopbptr->byte[COUNTH] = (UCHAR)(bufptr->b_bcount/BSIZE >> 8);

    data_addr = paddr(bufptr);
    
    /* Fill in the data address/modifier bytes. */

    put_addr(iopbptr, data_addr, DATA_ADDR);  
    iopbptr->byte[DATA_MOD] = STD_SUPER_DATA;

    iopbptr->buffer = bufptr;  /* The IOPB has to keep track of which  */
    			       /* buf structure it is related to here. */
}

/******************************************************************************
TITLE:		PUT_ADDR()
*******************************************************************************

DESCRIPTION:
		This function puts a 32 bit address into an IOPB's next IOPB
		address field or its data address field.
                                                                            
REQUIRES:                                                               
		An IOPB pointer.
		The 32 bit address to be stuffed.             
		Either a NEXT_IOPB, DATA_ADDR or LAST_IOPB flag.

SUBROUTINE CALLS:                                                       
		None.

EFFECTS:                                                                
		The appropriate field in the IOPB is updated.

ROUTINE RETURN VALUES:                                                  
		None.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL put_addr (iopbptr, address, where)
FAST IOPBPTR    iopbptr;
FAST ULONG      address;
FAST UCHAR      where;
{
    switch (where)
    {
        case NEXT_IOPB:  iopbptr->byte[NIOPB1] = (UCHAR)(address >> 24);
                         iopbptr->byte[NIOPB2] = (UCHAR)(address >> 16);
                         iopbptr->byte[NIOPB3] = (UCHAR)(address >> 8);
                         iopbptr->byte[NIOPB4] = (UCHAR)address;
                         break;

	case LAST_IOPB:
        case DATA_ADDR:  iopbptr->byte[DATA1] = (UCHAR)(address >> 24);
                         iopbptr->byte[DATA2] = (UCHAR)(address >> 16);
                         iopbptr->byte[DATA3] = (UCHAR)(address >> 8);
                         iopbptr->byte[DATA4] = (UCHAR)address;
                         break;
    }
}

/******************************************************************************
TITLE:		VALID_IOPB()
*******************************************************************************

DESCRIPTION:
		This function tries to match the IOPB pointer sent with both 
		the IOPB address array and the raw IOPB address array.  If it is
		found in either of those two arrays, the IOPB is valid and 
                the function returns TRUE.  Otherwise it is invalid, and FALSE
	 	is returned.
 
REQUIRES:                                                               
		A pointer to the IOPB, the IOPB address array, and the raw IOPB
		address array.

SUBROUTINE CALLS:                                                       
		None.

EFFECTS:                                                                
		None.

ROUTINE RETURN VALUES:                                                  
		RAW_IOPB if the IOPB is a valid raw IOPB.
		NORMAL_IOPB if the IOPB is a valid normal IOPB.
		FALSE if the IOPB is invalid.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL BOOL valid_iopb (iopbptr)
FAST IOPBPTR           iopbptr;
{
    FAST int index;

    for (index = 0; index < ICNT; ++index)
        if (iopbptr == iopb[index])
            return(TRUE);

    for (index = 0; index < UCNT; ++index)
        if (iopbptr == riopb[index])
            return(TRUE);

    return(FALSE);
}

/******************************************************************************
FILE = %W%	%G%	Inc.
*******************************************************************************
TITLE:		CONVERT_BLKNO()
*******************************************************************************

DESCRIPTION:
		This function takes a logical block number from a buf structure,
		converts it into appropriate cylinder, head, and sector values,
		and stuffs those values into an IOPB.
                                                                            
REQUIRES:                                                               
		A logical block number.
		A unit number so it can access the unit information table.
		A pointer to the IOPB.

SUBROUTINE CALLS:                                                       
		None.

EFFECTS:                                                                
		Writes conversion values into the IOPB passed to the function.

ROUTINE RETURN VALUES:                                                  
		None.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL convert_blkno (block, unit, iopbptr)
daddr_t              block;
USHORT               unit;
FAST IOPBPTR         iopbptr;
{
    UINT            cylinder, head, sector;
    FAST UNITINFO  *uinfo = &unit_table[unit];

    /* Compute the cylinder, head and sector values. */

    cylinder = block / (uinfo->sectors * uinfo->heads);
    head = (block - (cylinder * uinfo->sectors * uinfo->heads))/uinfo->sectors;
    sector = block - ((cylinder * uinfo->sectors * uinfo->heads) +
                      (head * uinfo->sectors));

    /* Then stuff the values into the IOPB. */

    iopbptr->byte[CYLL] = (UCHAR)(cylinder);
    iopbptr->byte[CYLH] = (UCHAR)(cylinder >> 8);
    iopbptr->byte[HEAD] = (UCHAR)head;
    iopbptr->byte[SECTOR] = (UCHAR)sector;
}

/******************************************************************************
FILE = %W%	%G%	Inc.
*******************************************************************************
TITLE:		OPEN()
*******************************************************************************

DESCRIPTION:
		This function is called when the device is mounted, or when
		a device special file for this device is opened explicitly.
		If the device is there and not busy, the device is marked
		OPEN.
                                                                            
REQUIRES:                                                               
		A device number from the operating system.
		The appropriate unit information table.

SUBROUTINE CALLS:                                                       
		None.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

m323open (devnum, flag, otyp)
dev_t   devnum;
int     flag;
int	otyp;
{
    USHORT      ctl, unit, part;
    IOPBPTR     iopbptr;
    UNITINFO   *uinfo;
    PARTINFO   *pinfo;
    BUFPTR      bufptr;
    int		*otbl;		/* address of open table */
    register struct iobuf *dp;

    ctl = CTL(devnum);
    unit = DRIVE(devnum);     /* Set up the local variables and pointers. */
    part = PARTITION(devnum);
    uinfo = &unit_table[unit];
    pinfo = (PARTINFO *)&(part_table[(unit * PCNT) + part]);
    otbl = &m323otbl[((ctl * UCNT) + unit) * OTYPCNT];

    /* If the unit number is too big, or if the device isn't there, then  */
    /* return ENXIO.  */
    if ((unit >= UCNT) || (pinfo->p_status IS ABSENT))
    {
        u.u_error = ENXIO;
	return;
    }

   /* check for open in progress */
	dp = &m323utab[POS(devnum)];

	while( dp->b_flags&B_COPEN ) {	/* open in progress? */
		dp->b_flags |= B_WANTED;
		sleep(dp,PRIBIO);
	}

#ifdef AUTOCONFIG

   /* Replace some default drive parameters with config info from disk
      if it is valid and it hasnt been done previously. 
   */
	if((autoconf) && (dp->b_flags & B_CONF))  {
		dp->b_flags |= B_COPEN;
		if( m323cf(devnum) ) {	 /* replace drive parameters */
			u.u_error = EIO;     /* cant read block 0 */
			dp->b_flags &= ~B_COPEN; /* abort open */ 
		}
		else
			dp->b_flags &= ~(B_CONF|B_COPEN); /* config & OPEN done */
		if(dp->b_flags & B_WANTED)
			wakeup(dp);
	}
#endif

   /* keep track of open type also */
   if(otyp == OTYP_LYR)
	++otbl[OTYP_LYR];
   else if( otyp < OTYPCNT)
	otbl[otyp] |= ( 1 << part);

   u.u_error = 0;
}


#ifdef AUTOCONFIG
/* Read configuration information from the disk. Update critical drive and
   format parameters based on the information. */
m323cf(dev)
dev_t   dev;
{
	register UNITINFO  *uinfo;
	register struct buf *bp;
	struct volumeid *vp;
	struct dkconfig *cfp;
	USHORT unit;
	IOPBPTR	iopbptr;

	unit = DRIVE(dev);

	/* get a buffer	for the	config data */
	bp = geteblk ();
	bp->b_flags  |=  B_ABS | B_READ;
	bp->b_error = 0;
	bp->b_proc = u.u_procp;
	bp->b_dev = dev;
	bp->b_blkno = 0;	/* reading absolute block zero */

	/* setup ptrs assuming vid is sector 0,	config area is sector 1	*/
	vp = (struct volumeid *)(bp->b_un.b_addr);
	cfp = (struct dkconfig *)((int)bp->b_un.b_addr + 256);

	/* send	to strategy */
	m323strategy (bp);
	iowait (bp);

	bp->b_flags  &= ~B_ABS;
	/* error check,	signature string */
	if (bp->b_flags & B_ERROR ) {
		brelse(bp);
		return(-1);
	}
	if (sigcheck(vp->vid_mac)) {

		/* Replace selected default drive parameters with info from 
	  	   configuration area on disk.
		*/
		uinfo = &unit_table[POS(dev)];
		uinfo->heads = cfp->ios_hds;
		uinfo->cylinders = cfp->ios_trk;
		uinfo->sectors = cfp->ios_spt;
		uinfo->sect_size = cfp->ios_psm;
		brelse(bp);

		/* ADD CODE HERE AT SOME FUTURE DATE TO ASSURE EXLCUSIVE USE OF
		   THE RAW IOPB FOR THE SPECIFIED DRIVE. */
		iopbptr = riopb[unit];		/* get raw IOPB */
		init_iopb(iopbptr, unit);	/* initialize the raw IOPB */
       		if (execute( BY_INTERRUPT, XYSETDRVPAR, dev ) IS_A FAILURE)
		{
			return(-1);
		}
	}
	brelse (bp);
	return(0);	/* no error */
}

LOCAL int sigcheck(str)  /* validate signature string in VID */
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
#endif
/******************************************************************************
FILE = %W%	%G%	Inc.
*******************************************************************************
TITLE:		PRINT()
*******************************************************************************

DESCRIPTION:
             USELESS BUT NECESSARY FOR SOME REASON 
                                                                            
REQUIRES:                                                               

SUBROUTINE CALLS:                                                       

EFFECTS:                                                                

ROUTINE RETURN VALUES:                                                  

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

m323print(dev)
int      dev;
{
}

/******************************************************************************
FILE = %W%	%G%	Inc.
*******************************************************************************
TITLE:		DEQ_BUF()
*******************************************************************************

DESCRIPTION:
		This function dequeues the next buf structure from 
		the BUF queue.
                                                                            
REQUIRES:                                                               
		Nothing.

SUBROUTINE CALLS:                                                       
		None.

EFFECTS:                                                                
		Re-arranges pointers in BUF queue head node.

ROUTINE RETURN VALUES:                                                  
		Pointer to dequeued buf structure.
		EMPTY if the queue was empty.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL BUFPTR deq_buf()
{
    FAST BUFPTR     bufptr;

    if ((bufptr = Queue.buffs->av_forw) IS_NOT EMPTY ) 
    {
        /* If the head node "forward" pointer is not EMPTY, then there is a */
        /* buf structure available.  Reset the head node "forward" pointer  */
        /* to equal the "forward" pointer of the buf currently on top of   */
        /* the BUF queue, thereby removing the buf from the linked list.   */   
    
        Queue.buffs->av_forw = bufptr->av_forw;
        
        /* If the dequeued buf was the only one on the queue, then its */
        /* forward pointer would be EMPTY. If so, set the head node's  */
        /* "back" pointer to EMPTY, thereby flushing the queue.        */
    
        if (Queue.buffs->av_forw IS EMPTY) 
            Queue.buffs->av_back = EMPTY;   
    }
    
    /* Return either the address of the dequeued buf, */
    /* or if the queue was empty, return EMPTY.       */
     
    return(bufptr);
}

/******************************************************************************
TITLE:		DEQ_IOPB()
*******************************************************************************

DESCRIPTION:
		This function removes the top IOPB from a queue.
                                                                            
REQUIRES:                                                               
		A pointer to the queue.

SUBROUTINE CALLS:                                                       
		put_addr() - Puts a data or "next" IOPB address into an IOPB.
		get_addr() - Gets a data or "next" IOPB address into an IOPB.

EFFECTS:                                                                
		Adjusts pointers of the queue head and the affected IOPBs
		on the queue.

ROUTINE RETURN VALUES:                                                  
		A pointer to the dequeued IOPB.
		EMPTY if the queue is empty.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL IOPBPTR  deq_iopb (queueptr)
IOPBPTR                  queueptr;
{
    ULONG      address, get_addr();
    IOPBPTR    iopbptr;

    /* If the "next" IOPB pointer of the head node is not EMPTY, then there */
    /* is an IOPB on the chain.  Turn OFF the Chain Enable bit of the IOPB  */
    /* on top before taking it.  Then get the "next" IOPB address from the */
    /* IOPB to be dequeued, and put it into the "next" IOPB pointer of the */
    /* head node, thereby making it the new "top of the queue" IOPB.       */
    
    if ((iopbptr = (IOPBPTR)get_addr(queueptr, NEXT_IOPB)) IS_NOT EMPTY)
    {
        iopbptr->byte[COMMAND] &= ~CHEN;     
        address = get_addr(iopbptr, NEXT_IOPB);
        put_addr(queueptr, address, NEXT_IOPB);
    
        /* If the queue was emptied, set the "back" pointer of */
        /* the head node to EMPTY, thereby flushing the queue. */
    
        if (address IS EMPTY)     
            put_addr(queueptr, (ULONG)NULL, LAST_IOPB);
    }
    
    /*  Return the value returned by get_addr():  */ 
    /*    1) The address of the dequeued IOPB, or  */
    /*    2) EMPTY if the queue was empty.         */
    
    return(iopbptr);
}

/******************************************************************************
TITLE:		ENQ_BUF()
*******************************************************************************

DESCRIPTION:
		This function enqueues a buf structure from the operating 
                system onto the driver's BUF queue.  
                                                                            
REQUIRES:                                                               
		A buf pointer.

SUBROUTINE CALLS:                                                       
		None.

EFFECTS:                                                                
		Re-arranges pointers in BUF queue head node and any affected
		buf structure already on the queue.

ROUTINE RETURN VALUES:                                                  
		None.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL enq_buf (bufptr)
FAST BUFPTR    bufptr;
{

    bufptr->av_forw = EMPTY;  /* Set the buffer's "forward" pointer to EMPTY. */

    if (Queue.buffs->av_forw IS EMPTY)  
    {
        /* If the head node's "forward" pointer is EMPTY, then */
        /* the BUF queue is empty.  Set the "forward" pointer  */
        /* of the head node to point to the buf struture sent. */
    
        Queue.buffs->av_forw = bufptr;   
    }
    else
    {
        /* Otherwise, set the "forward" pointer of the last buf */
        /* in the queue to point to the new buf structure.      */
    
        Queue.buffs->av_back->av_forw = bufptr; 
    }
    
    /* Set the "back" pointer of the head node */
    /* to point to the new buf structure.      */
    
    Queue.buffs->av_back = bufptr;
}

/******************************************************************************
TITLE:		ENQ_IOPB()
*******************************************************************************

DESCRIPTION:
		This function enqueues an IOPB onto a queue.
                                                                            
REQUIRES:                                                               
		A flag indicating which queue to use.
		A pointer to the IOPB.

SUBROUTINE CALLS:                                                       
		put_addr() - Puts a data or "next" IOPB address into an IOPB.
		get_addr() - Gets a data or "next" IOPB address into an IOPB.

EFFECTS:                                                                
		Adjusts pointers of the queue head and the affected IOPBs
		on the queue.

ROUTINE RETURN VALUES:                                                  
		None.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL enq_iopb (queueptr, iopbptr)
FAST IOPBPTR    queueptr, iopbptr;
{
    IOPBPTR    lastiopb;
    ULONG      get_addr();

    /* If the "next" IOPB pointer of the head node is not EMPTY, then */
    /* the queue is not empty, and the IOPB must be added to the end  */
    /* of the queue.  If it is NULL, then the IOPB is just added to   */
    /* the top of the queue.                                          */
    
    if (get_addr(queueptr, NEXT_IOPB) IS_NOT EMPTY)      
    {
        /* Get the address of the last IOPB on the queue. */
    
        lastiopb = (IOPBPTR)(get_addr(queueptr, LAST_IOPB));
    
        /* Set the Chain Enable bit in the last IOPB on the queue.   */
        /* Then put the address of the new IOPB into the "next" IOPB */
        /* field of the last IOPB on the queue.  The new IOPB then   */
        /* becomes the last IOPB on the chain.                       */    
    
        lastiopb->byte[COMMAND] |= CHEN;
	lastiopb->byte[IOPB_MOD] = STD_SUPER_DATA;
        put_addr(lastiopb, (ULONG)iopbptr, NEXT_IOPB);
    }
    else                                                
    {
        /* The queue is empty, so just add the */ 
        /* new IOPB to the top of the queue.   */
    
        put_addr(queueptr, (ULONG)iopbptr, NEXT_IOPB);
    }
    
    /* Finally, set the "next" IOPB field of the new IOPB to EMPTY. */
    /* and set the "back" pointer of the head node to point to the */
    /* IOPB that was just added to the queue.                      */
    
    put_addr(iopbptr, (ULONG)NULL, NEXT_IOPB);           
    put_addr(queueptr, (ULONG)iopbptr, LAST_IOPB);
    
}

/*****************************************************************************
TITLE:		FLUSH_QUEUE()
*******************************************************************************

DESCRIPTION:
		This function stuffs NULLs into the data and next IOPB 
		address fields of the appropriate IOPB queue head node. 
                                                                            
REQUIRES:                                                               
		A pointer to the IOPB queue.

SUBROUTINE CALLS:                                                       
		put_addr() - Puts a data or "next" IOPB address into an IOPB.

EFFECTS:                                                                
		Effectively flushes a queue.

ROUTINE RETURN VALUES:                                                  
		None.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL flush_queue (queueptr)
IOPBPTR            queueptr;
{
    put_addr(queueptr, (ULONG)NULL, NEXT_IOPB);
    put_addr(queueptr, (ULONG)NULL, LAST_IOPB);
}

/******************************************************************************
TITLE:		INIT_QUEUES()
*******************************************************************************

DESCRIPTION:
		This function initializes the queue head nodes of the FREE
		and READY IOPB queues, and the BUF queue.  Then it allocates
		ICNT IOPBs and queues them onto the FREE IOPB queue, and UCNT 
		IOPBs and puts them into the raw IOPB array.
                                                                            
REQUIRES:                                                               
		Nothing.

SUBROUTINE CALLS:                                                       
		enq_iopb() - Enqueues an IOPB onto a specified queue.
		flush_queue() - Flushes a queue.
		rmalloc() - Dynamically allocates memory. (system)

EFFECTS:                                                                
		Sets up queue structures and allocates IOPB data space.

ROUTINE RETURN VALUES:                                                  
		None.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL init_queues()
{
    FAST USHORT     index;

    Boot_Flag = NO;     /* Clear the boot flag on initial probe() call. */

    Queue.ready = &ready_head;     /* Initialize the driver's master */
    Queue.free = &free_head;       /* queue pointer structure.       */
    Queue.buffs = &buf_head;
    
    flush_queue(Queue.free);       /* Flush both of the IOPB queues. */
    flush_queue(Queue.ready);

    Queue.buffs->av_forw = EMPTY;   /* Flush the BUF queue. */
    Queue.buffs->av_back = EMPTY;

    /* Allocate ICNT IOPBs and enqueue them onto the FREE IOPB queue. */
    
    for (index = 0; index < ICNT; ++index)    
    {
        iopb[index] = &io_vec[ index ];
        enq_iopb(Queue.free, iopb[index]);
    }
    
    /* Allocate UCNT IOPBs and put them into the raw IOPB array. */
    
    for (index = 0; index < UCNT; ++index)  
        riopb[index] = &io_vec[ ICNT + 1 ];
    
    /* Initialize the buf structure array. */

    for (index = 0; index < UCNT; ++index)  
        rbuf[index].b_flags = CLEAR;

}

/******************************************************************************
FILE = %W%	%G%	Inc.
*******************************************************************************
TITLE:		READ()
*******************************************************************************

DESCRIPTION:
		This character interface routine is called to transfer data
		directly from the device to user memory.  A buffer is
		sent to the system call physio(), which in turn locks in 
		the user data and calls strategy().

REQUIRES:                                                               
		A valid device number.
                A buf structure.

SUBROUTINE CALLS:                                                       
		physio() - Sets up the buf structure sent to it to look 
		    like a system buf structure, locks in the user data area,
		    and then calls strategy(). (system)

EFFECTS:                                                                
		Temporarily utilizes the global read_buf buf structure. 

ROUTINE RETURN VALUES:                                                  
		ENXIO - If device is non-existent.
		Return value from physio() is returned by this routine.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

m323read (dev)
FAST int     dev;
{
	int	m323breakup();		
	FAST PARTINFO   *pinfo;

	pinfo = (PARTINFO *)&(part_table[(DRIVE(dev) * PCNT) + PARTITION(dev)]);

	if(physck(pinfo->length,B_READ))
		physio(m323breakup, 0, dev, B_READ);
}

/******************************************************************************
FILE = %W%	%G%	Inc.
*******************************************************************************
TITLE:		CLEAR_BIT()
*******************************************************************************

DESCRIPTION:
		This function clears a requested bit in the CSR if it is
		currently set, thereby avoiding violation of the register 
		protocol.
                                                                            
REQUIRES:                                                               
		A bit offset in the CSR

SUBROUTINE CALLS:                                                       
		read_reg() - Reads a controller register.
		write_reg() - Writes to a controller register.

EFFECTS:                                                                
		The reading of the CSR will set the RBS bit.

ROUTINE RETURN VALUES:                                                  
		None.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL clear_bit (bit)
FAST UCHAR       bit;
{
    /* Only clear a bit if it is currently set. */

    if ((read_reg(CSR) & bit) IS_NOT CLEAR) 
        write_reg(CSR, bit);
}

/******************************************************************************
TITLE:		READ_ADDR()
*******************************************************************************

DESCRIPTION:
		This function reads the controller's address registers and
		returns a pointer to a completed IOPB.

REQUIRES:                                                               
		Nothing.

SUBROUTINE CALLS:                                                       
		read_reg() - Reads a controller register.

EFFECTS:                                                                

ROUTINE RETURN VALUES:                                                  

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL IOPBPTR read_addr()
{
    FAST ULONG big_addr = 0;        /* 32 Bit Address */

    /* Assemble the 32 bit address from the controller registers. */

    big_addr |= (ULONG)read_reg(ADDR_4) << 24;
    big_addr |= (ULONG)read_reg(ADDR_3) << 16;
    big_addr |= (ULONG)read_reg(ADDR_2) << 8;
    big_addr |= (ULONG)read_reg(ADDR_1);

    return((IOPBPTR)big_addr);
}

/******************************************************************************
TITLE:		READ_REG()
*******************************************************************************

DESCRIPTION:
		This function simply reads the controller register requested.
                                                                            
REQUIRES:                                                               
		Register offset.

SUBROUTINE CALLS:                                                       
		None.

EFFECTS:                                                                
		Reading CSR will set RBS bit.

ROUTINE RETURN VALUES:                                                  
		Byte value read.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL read_reg(offset)
FAST UCHAR      offset;
{
    switch (offset)
    {
        case ADDR_1:   return ((UCHAR)ctlr_table.reg_address->addr1);

        case ADDR_2:   return ((UCHAR)ctlr_table.reg_address->addr2);

        case ADDR_3:   return ((UCHAR)ctlr_table.reg_address->addr3);

        case ADDR_4:   return ((UCHAR)ctlr_table.reg_address->addr4);

        case MODIFIER: return ((UCHAR)ctlr_table.reg_address->modifier);

        case CSR:      return ((UCHAR)ctlr_table.reg_address->csr);

        case FATAL:    return ((UCHAR)ctlr_table.reg_address->fatal);
    }
}

/******************************************************************************
TITLE:		SET_BIT() 
*******************************************************************************

DESCRIPTION:
		This function sets a requested bit in the CSR if it is NOT
		currently set, thereby avoiding violation of the register 
		protocol.
                                                                            
REQUIRES:                                                               
		A bit offset in the CSR

SUBROUTINE CALLS:                                                       
		read_reg() - Reads a controller register.
		write_reg() - Writes to a controller register.

EFFECTS:                                                                
		The reading of the CSR will set the RBS bit.

ROUTINE RETURN VALUES:                                                  
		None.
                                                                            
*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL set_bit (bit)
FAST UCHAR     bit;
{
    /* Set the bit only if it is currently clear. */

    if ((read_reg(CSR) & bit) IS CLEAR)
        write_reg(CSR, bit);
}

/******************************************************************************
TITLE:		STUFF_ADDR()
*******************************************************************************

DESCRIPTION:
		This function loads the controller's address registers.
		It receives either the READY_QUEUE flag (NULL) from go(),
		or the address of a discrete IOPB from execute().
		First, however, it has to strip the DVMA factor from the 
		"next" IOPB address in the READY IOPB queue head node, 
		as well as in any IOPB in the chain that points to a 
		"next" IOPB.  By doing so, the controller can directly DMA 
		the IOPB(s).
                                                                            
REQUIRES:                                                               
		A pointer to an IOPB.

SUBROUTINE CALLS:                                                       
                put_addr() - Writes an address into an IOPB.
		get_addr() - Returns an address from an IOPB.
		write_reg() - Writes to a controller register.

EFFECTS:                                                                
		Allows controller to directly DMA IOPBs.
		Puts controller in a state where it needs only the AIO bit
		to be set in order to take off.

ROUTINE RETURN VALUES:                                                  
		None.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL stuff_addr (iopbptr)
IOPBPTR           iopbptr;
{
    IOPBPTR       current, next;
    FAST ULONG    big_addr;        /* 32 bit address */
    ULONG         get_addr();
    CTLRINFO     *cinfo = &ctlr_table;


    if (iopbptr POINTS_TO READY_QUEUE) 
    {
        for (current = Queue.ready;
             (next = (IOPBPTR)get_addr(current, NEXT_IOPB)) IS_NOT NULL;)
        {
            put_addr(current, (ULONG)next, NEXT_IOPB);

            current = next;
	    if (cinfo->param1 & ICS) 
	        make_checksum(current);
        }

        big_addr = get_addr(Queue.ready, NEXT_IOPB);
    }
    else    /* Either it's a polled command or a RAW IOPB. */
    {
        big_addr = (ULONG)iopbptr;   
    }
    write_reg(ADDR_4, (UCHAR)(big_addr >> 24));
    write_reg(ADDR_3, (UCHAR)(big_addr >> 16));
    write_reg(ADDR_2, (UCHAR)(big_addr >> 8));
    write_reg(ADDR_1, (UCHAR)(big_addr));
    write_reg(MODIFIER, STD_SUPER_DATA);
}

/******************************************************************************
TITLE:		TEST_BIT()
*******************************************************************************

DESCRIPTION:
		This function tests a requested bit in the CSR register, and
		returns SET or CLEAR to the caller.
                                                                            
REQUIRES:                                                               
		A bit number in the CSR.

SUBROUTINE CALLS:                                                       
		read_reg() - Reads a controller register.

EFFECTS:                                                                
		By reading the CSR, RBS will become set.

ROUTINE RETURN VALUES:                                                  
		SET if bit is set, or
		CLEAR if bit is clear.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL test_bit (bit)
FAST UCHAR      bit;
{
    if (read_reg(CSR) & bit)
        return(SET);
    else
        return(CLEAR);
}

/******************************************************************************
TITLE:		WRITE_REG()
*******************************************************************************

DESCRIPTION:
		This function simply writes the byte value sent to the
		the controller register requested.
                                                                            
REQUIRES:                                                               
		Register offset and value to be written.

SUBROUTINE CALLS:                                                       
		None.

EFFECTS:                                                                
		See the Controller Manual for effects of writing to the 
                CSR register.

ROUTINE RETURN VALUES:                                                  
		None.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/
 
LOCAL write_reg (offset, value)
FAST UCHAR       offset, value;
{
    switch (offset)
    {
        case ADDR_1:   ctlr_table.reg_address->addr1 = value;
                       break;

        case ADDR_2:   ctlr_table.reg_address->addr2 = value;
                       break;

        case ADDR_3:   ctlr_table.reg_address->addr3 = value;
                       break;

        case ADDR_4:   ctlr_table.reg_address->addr4 = value;
                       break;

        case MODIFIER: ctlr_table.reg_address->modifier = value;
                       break;

        case CSR:      ctlr_table.reg_address->csr = value;
                       break;
    }
}

/******************************************************************************
FILE = %W%	%G%	Inc.
*******************************************************************************
TITLE:		STRATEGY()
*******************************************************************************

DESCRIPTION:
		This function accepts buf structures from the operating system,
		and takes the steps necessary to effect the required data 
		transfer.  After checking for the sanity of the buf structure 
		sent, it raises its priority to that of the interrupt handler.
		It then queues the buf structure on the BUF queue, and calls
		buf_to_iopb().  If the function returns TRUE, meaning that 
		there is an available IOPB structure(s) to execute, then go()
		is called to send the IOPB to the controller.

REQUIRES:                                                               
		A pointer to a buf structure.

SUBROUTINE CALLS:                                                       
		iodone() - Returns a buf to the operating system. (system)      
		enq_buf() - Enqueues a buf structure onto the BUF queue.
		buf_to_iopb() - Attempts to make IOPBs out of bufs.    
		go() - Sets controller in motion with READY IOPB Queue.

EFFECTS:                                                                
		If buf structure is incorrect, calls iodone() here.
		Queues the buf structure onto the BUF queue.
		Temporarily locks out interrupts from this device.
		Attempts to get an IOPB to associate with the buf structure.
		If an IOPB is ready, sends it to the controller.
		
ROUTINE RETURN VALUES:                                                  
		None.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/
/* Physical I/O breakup routine */
m323breakup(bp)
register struct buf *bp;
{
	dma_breakup(m323strategy, bp, BSIZE);
}


m323strategy(bufptr)
FAST BUFPTR      bufptr;
{
    FAST UNITINFO     *uinfo;
    FAST PARTINFO     *pinfo;
    FAST USHORT        unit, part;
    FAST ULONG         last;
    USHORT    old_priority;
    register struct iotime *ip;

    unit = DRIVE(bufptr->b_dev);        /* Set up local variables. */
    part = PARTITION(bufptr->b_dev);
    uinfo = &unit_table[unit];
    pinfo = (PARTINFO *)&(part_table[(unit*PCNT) + part]);


    if (unit >= UCNT)           /* If the unit number is too big */
    {
        bufptr->b_flags |= B_ERROR;     
        bufptr->b_error = ENXIO;   /*  "non-existant" device */
	bufptr->b_resid = bufptr->b_bcount;
        iodone(bufptr);
        return;
    }
    
    if (pinfo->p_status == ABSENT) 
    {
        dri_printf("M323: Unit %d Not Operational\n",unit);
        bufptr->b_flags |= B_ERROR;    /*  unit isn't operational */
        bufptr->b_error = ENXIO;   /*  "non-existant" device */
	bufptr->b_resid = bufptr->b_bcount;
        iodone(bufptr);
        return;
    }
    
    /* If this is a raw buf structure, the b_bcount field is used */
    /* as a pointer to a user-created IOPB.  Therefore, the next  */
    /* test is irrelevant to raw bufs.                            */

    if (! RAWBUF(bufptr))
    {
	/* no partial disk block i/o */
	if(bufptr->b_bcount&(BSIZE-1))
    	{
		bufptr->b_flags |= B_ERROR;
        	bufptr->b_error = ENXIO;   /*  "non-existant" device */
		bufptr->b_resid = bufptr->b_bcount;
       		iodone(bufptr);
	        return;
    	}

	last = bufptr->b_blkno;
    
	if(last >= pinfo->length) 
	{
		if((last == pinfo->length) && (bufptr->b_flags&B_READ))
		{
			bufptr->b_resid = bufptr->b_bcount;
		}
		else
		{
			/*  request over-runs partition, */
      			bufptr->b_flags |= B_ERROR;  
		}
        	iodone(bufptr);
	        return;
	}
    }

    /* log I/O statistics for this drive */
    ip = &m323stat[POS(bufptr->b_dev)];
    if( ( bufptr->b_flags & NON_RWIO) )
	++(ip->io_other);
    else {
	++(ip->io_cnt);	
	ip->io_bcnt += bufptr->b_bcount/BSIZE;
    }

   /* Convert the Ctlr's hardware priority level to a CPU priority level, and */
   /* temporarily assign it to this routine.  This will effectively lock out  */
   /* interrupts while data and registers that are common to the interrupt    */
   /* service routine are accessed.  The old priority level will be stored    */
   /* in "old_priority".                                                      */

    old_priority = SPL( INTR_PRIORITY );
        
    enq_buf(bufptr);    /* Enqueue the buf structure onto the BUF queue. */


    /* If there are any IOPBs on the FREE IOPB queue, dequeue them, and   */
    /* make new IOPBs for as many buf structures as are on the BUF queue. */
    /* Queue them onto the READY IOPB queue, and dispatch them to the     */
    /* controller by calling go().                                        */

    if (buf_to_iopb())     
        go();               

    splx(old_priority);    /* Restore the former CPU priority level. */
}

/******************************************************************************
FILE = %W%	%G%	Inc.
*******************************************************************************
TITLE:		WRITE()
*******************************************************************************

DESCRIPTION:
		This character interface routine is called to transfer data
		directly from user memory to the device.  A buffer is
		sent to the system call physio(), which in turn locks in 
		the user data and calls strategy().
                                                                            
REQUIRES:                                                               
		A valid device number.
		A buf structure.

SUBROUTINE CALLS:                                                       
		physio() - Sets up a the buf structure sent to it to look 
		like a system buf structure, locks in the user data area,
		and then calls strategy(). (system)

EFFECTS:                                                                
		Temporarily utilizes the global write_buf buf structure.

ROUTINE RETURN VALUES:                                                  
		ENXIO - If device is non-existent.
		Return value from physio() call is returned by this routine.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

m323write (dev)
FAST int      dev;
{
    int	m323breakup();
    FAST PARTINFO   *pinfo;

    pinfo = (PARTINFO *)&(part_table[(DRIVE(dev) * PCNT) + PARTITION(dev)]);

	if(physck(pinfo->length,B_WRITE))
        	physio(m323breakup, 0, dev, B_WRITE);
}


