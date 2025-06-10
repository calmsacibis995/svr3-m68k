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


