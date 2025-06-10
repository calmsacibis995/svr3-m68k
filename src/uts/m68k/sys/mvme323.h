/*	@(#)mvme323.h	7.5 	*/
/*
 *	Copyright (c) 1987	Motorola Inc.  
 *		All Rights Reserved.
 */
/*
 *	mvme323.h: include file for Motorola MVME323 disk controller
 */

#define	M323CTLS 1	/* Driver currently supports only one controller */
#define	M323DRV	4	/* maximum number of drives */
#define M323SLICE 8     /* PARTITION COUNT  */
#define M323ICNT 10     /* IOPB COUNT       */

#define FLUSHLIM 30	/* Allow controller to empty its IOPB queue after
			   sending it 30 IOPBs. */

struct m323size {
    unsigned char   p_status;    
    unsigned int    first;      
    unsigned int    length;    
};
typedef struct m323size PARTINFO;

/*
 *	Configuration Control: Only needs certain fields from dkconfig
 */
struct m323config {
         ushort		cf_cyls;	/* number of cylinders */
         unsigned char	cf_vnh;		/* volume number of heads */
         unsigned char	cf_spt;		/* sectors per track */
         ushort		cf_bps;		/* bytes per sector */
         unsigned char	cf_intlv;	/* sector interleave factor */
         unsigned char	cf_gap1;	/* words in gap 1 */
         unsigned char	cf_gap2;	/* words in gap 2 */
         ushort		cf_attrib;	/* attributes */
};

/*
 *	configuration attribute flags & masks
 *      are currently unassigned.
 */

struct m323eregs {
/*  0 */ unsigned int	er_blkoff;	/* block offset of error */
/*  4 */ unsigned char	er_cmd;		/* command and status summary */
/*  5 */ unsigned char	er_cmplcode;	/* completion code */
/*  6 */ unsigned char	er_st2; 	/* status byte 2 (drive status) */
/*  7 */ unsigned char	head;		/* head */	
/*  8 */ unsigned short	cyl;		/* cylinder */	
};

#define M323EREG	(sizeof(struct m323eregs)/sizeof(short))






/*
*******************************************************************************

		This header file contains the configuration constants
		plus assorted other definitions that are local to the
		driver code.
                                                                            
*******************************************************************************
*/
/* Assorted driver definitions. */
 
#define ISIZE             0x1e     /* Standard VME Ctlr IOPB Size             */
#define STD_SUPER_DATA    0x3d     /* Standard Supervisory Data.              */
#define B_RAWBUF          B_ERROR  /* Mask for raw bufs in b_flags field.     */
#define DISK              0        /* Drive type for WHATAMI ioctl() call.    */
#define OPENED            0        /* Successfull return from open() call.    */
#define SERVICED          1        /* Interrupt serviced.                     */
#define NOT_SERVICED      0        /* Interrupt not serviced.                 */
#define DISABLED          0        /* For interrupt level field of IOPB.      */
#define FIRST_PARTITION   0        /* First partition table in array.         */
#define BUS_ERROR       (-1)       /* Bus error return from peekc() call.     */
#define NOT_THERE         0        /* probe() return if Ctlr is not there.    */
#define IOCTL_OK          0        /* Call to ioctl() succeeded.              */
#define IOCTL_FAILED    (-1)       /* Call to ioctl() failed.                 */
#define POLLED            1        /* Argument to  execute().                 */
#define BY_INTERRUPT      2        /* Argument to  execute().                 */
#define ONE_TRACK         1        /* Number of tracks to format.             */
#define FORMAT_PARMS      13       /* Size of format parameters table.        */
#define F_PAR_STAT        12       /* Status byte in format parameters table. */
#define F_PAR_OK          0        /* Status byte value if IOPB worked OK.    */
#define DEFECT_MAP_SIZE   266      /* Bytes Returned on Read Defect Map Cmd   */
#define WORK_BUFF_SIZE (ULONG)4096 /* Ioctl call work buffer size. 4K         */
#define READY_QUEUE ((IOPBPTR)NULL)/* Flag used by stuff_addr() to differen-  */
                                   /* tiate between the use of the READY IOPB */
                                   /* queue or the use of a polled/RAW IOPB.  */
#define DATA_ADDR          0       /* Signals use of Data address bytes. */
#define LAST_IOPB          1       /* Signals use of Last IOPB Bytes.    */
#define NEXT_IOPB          2       /* Signals use of Next IOPB bytes.    */
#define INT_LEVEL_MASK   0xf0      /* Interrupt level mask for byte 6.   */

/* UNIT INFORMATION STATUS. */
#define U_OFFLINE      BIT0
#define U_ONLINE       BIT1

/* PARTITION STATUS. */
#define ABSENT         0
#define PRESENT        1

#define SOFTERR 0 	/* soft error */	
#define HARDERR 1	/* hard error */


/******************************************************************************

		This header file contains the current set of ioctl commands
        	and the structure format of the ioctl table to be passed
		from the user to the ioctl() routine.
                                                                            
*******************************************************************************/

/*  IOCTL COMMAND SET  */

#define XYWHATAMI     1      
#define XYREVNO       2
#define XYCTLRINFO    3
#define XYUNITINFO    4
#define XYSETCTLRPAR  5
#define XYSETDRVPAR   6
#define XYRDCTLRPAR   7
#define XYRDDRVPAR    8
#define XYDRVRESET    9

#define XYOPTIONAL    18
#define XYPARTINFO    20
#define XYSETPARTS    21
#define XYFORMAT      22
#define XYRDTRKHDR    23
#define XYWRTRKHDR    24
#define XYREAD        25
#define XYWRITE       26
#define XYRDFORMPAR   27
#define XYSETFORMPAR  28
#define XYRESETCTLR   29
#define XYRDDEFECTS   30
#define XYDRVCONFIG   31


/******************************************************************************

DESCRIPTION:
		This header file includes the macros used by the driver.
                                                                            
*******************************************************************************/

#define CTLR(dev)	      0		/* support for controller zero only */ 
#define DRIVE(dev)            ((minor(dev)&0x30) >> 4) 
#define POS(dev)	      ((CTLR(dev) * M323DRV) + DRIVE(dev))
#define PARTITION(dev)        (minor(dev)&0x0F)  

#define TEST_(x)              test_bit(x)
#define CLEAR_(x)             clear_bit(x)
#define SET_(x)               set_bit(x)

#define RAWBUF(bufptr)        (bufptr->b_flags & B_RAWBUF)

#define INTR_PRIORITY         unit_table[DRIVE(bufptr->b_dev)].int_level
#define SPL(x)                (*intr_lock[x].function)()


#define COPY_IN( arg, local_table )                                           \
                                                                              \
            status = copyin( arg, &local_table.status, sizeof(IOCTL_TABLE));  \
                                                                              \
            if ( status == -1 )                                               \
                {                                                             \
                printf("COPYIN FAILED\n");                                    \
                u.u_error = EFAULT;                                           \
                return(IOCTL_FAILED);                                         \
                }

#define COPY_OUT( arg, local_table )                                          \
                                                                              \
            status = copyout( ioctl_table, arg, sizeof( IOCTL_TABLE ));       \
                                                                              \
            if ( status == -1 )                                               \
                {                                                             \
                u.u_error = EFAULT;                                           \
                printf("COPYOUT FAILED\n");                                   \
                return(IOCTL_FAILED);                                         \
                }

/* Macros used to decode Error Numbers into Error Types */

#define PROG_ERROR(x)          ( (x >> 4) == 0x1 )
#define SOFT_ERROR(x)          ( (x >> 4) == 0x3 )
#define RETRY_HARD(x)          ( (x >> 4) == 0x4 )
#define RESET_RETRY(x)         ( (x >> 4) == 0x6 )
#define NO_RETRY(x)            ( (x >> 4) == 0x7 )
#define MISC_ERROR(x)          ( (x >> 4) == 0x8 )
#define INTERVENTION(x)        ( (x >> 4) == 0x9 )
#define YOUR_LOST(x)           ( (x >> 4) == 0xa )


/******************************************************************************

DESCRIPTION:
		This header file contains some generic definitions that
		don't fit into any other category, but are used everywhere.
                                                                            
*******************************************************************************/

typedef  unsigned char    UCHAR, BYTE, BOOL; /* 8 Bits unsigned.  */
typedef  unsigned short   USHORT;            /* 16 Bits unsigned. */
typedef  unsigned int     UINT;              /* System dependent. */
typedef  unsigned long    ULONG;             /* 32 Bits Unsigned. */

typedef  struct buf       BUF, *BUFPTR; 

#define  FAST             register
#define  LOCAL            static

/* Bit Definitions. */

#define BIT0       0x01
#define BIT1       0x02
#define BIT2       0x04
#define BIT3       0x08
#define BIT4       0x10
#define BIT5       0x20
#define BIT6       0x40
#define BIT7       0x80

/* Boolean Definitions. */

#define NO           0
#define BAD          0 
#define CLEAR        0
#define FAILURE      0
#define FALSE        0
#define EMPTY        0

#define YES          1
#define GOOD         1
#define SET          1
#define SUCCESS      1
#define SUCCESSFULL  1
#define TRUE         1

/*  Miscellaneous Definitions. */

#define WAIT_HERE  ;
#define NOT        !
#define IS_NOT     !=
#define IS         ==
#define IS_A       ==
#define POINTS_TO  ==
#define CAUSES_A   ==

/******************************************************************************

DESCRIPTION:
		This header file contains various controller, unit, driver,
		and IOPB status byte/bit definitions.
                                                                            
*******************************************************************************/
 
              /* GENERIC DISK DRIVE STATUS */

/* IOPB COMMAND BYTE. */

#define SGM            BIT4          /* Scatter/Gather Enable. */
#define CHEN           BIT5          /* Chain Enable.          */
#define DONE_BIT       BIT6          /* Done Bit.              */
#define ERRS           BIT7          /* Error Summary Bit.     */


/* IOPB COMPLETION CODES. */

#define Success                  0x00

/* Non-Retryable Programming Errors. */

#define Ill_Cylinder             0x10
#define Ill_Head                 0x11
#define Ill_Sector               0x12
#define Sector_Cnt_0             0x13
#define Ill_Cmd                  0x14
#define Ill_F1                   0x15
#define Ill_F2                   0x16
#define Ill_F3                   0x17
#define Ill_F4                   0x18
#define Ill_F5                   0x19
#define Ill_F6                   0x1a
#define Ill_F7                   0x1b
#define Ill_SG_Leng              0x1c
#define Short_Track              0x1d  /* Not Enough Sectors per Track */
#define Align_Err                0x1e  /* Next IOPB Alignment Error */

/* Successfully Recovered Soft Errors. */

#define ECC_Corrected            0x30
#define ECC_Ignored              0x31
#define Seek_Retry_Recovered     0x32
#define Retry_Recovered          0x33

/* Hard Errors - Retry. */

#define Hard_Data_Error          0x40
#define Header_Not_Found         0x41
#define Drive_Not_Ready          0x42
#define Operation_Timeout        0x43
#define DMAC_Timeout             0x44
#define Disk_Sequencer_Error     0x45
#define Buffer_Parity            0x46
#define Header_ECC_Err           0x48
#define Read_Verify              0x49
#define Fatal_DMAC_Err           0x4a
#define VMEbus_Err               0x4b

/* Hard Errors - Reset and Retry. */

#define Drive_Fault              0x60
#define HDR_Cyl_Error            0x61
#define HDR_Head_Error           0x62
#define Seek_Error               0x64

/* Hard Errors - Fatal. */

#define Illegal_Sect_Sz          0x70
#define Firmware_Error           0x71

/* Miscellaneous Errors. */

#define Soft_ECC                 0x80
#define IRAM_CS_Fail             0x81
#define Abort_Cmd                0x82

/* Manual Intervention Required */

#define Wrt_Protected            0x90

/* DISK STATUS - BYTE 2. */

#define DRDY           BIT0
#define SR             BIT6

/* DISK STATUS - BYTE 5. */

#define BTH            BIT4          /* Black Hole Transfer. */
#define FIXD           BIT7          /* Fixed Surface.       */

/* DRIVE PARAMETERS MODIFICATION - BYTE 6. */

#define EC32           BIT4          /* 48/32 Bit ECC.        */
#define ESD            BIT6          /* Embedded Servo Drive. */
#define AFE            BIT7          /* Mixed Formats Enable. */

/* CONTROLLER PARAMETERS MODIFICATION - BYTE 8. */

#define AIOR          (BIT0 | BIT1)  /* AIO Response Time.            */
#define NPRM           BIT2          /* Non-privilaged Register Mode. */
#define ICS            BIT4          /* IOPB Checksum Enable.         */
#define TMOD           BIT6          /* Transfer Mode.                */
#define AUD            BIT7          /* Auto Update.                  */

/* CONTROLLER PARAMETERS MODIFICATION - BYTE 9. */

#define ROR            BIT4          /* Release On Request. */
#define TDT           (BIT6 | BIT7)  /* Throttle Dead Time. */

/* CONTROLLER PARAMETERS MODIFICATION - BYTE A. */

#define ECCM          (BIT0 | BIT1)  /* ECC Mode.                */
#define RBC            BIT2          /* Retry Before Correction. */
#define ZLR            BIT3          /* Zero Latency Reads.      */
#define ASR            BIT4          /* Auto Seek Retry.         */
#define COP            BIT6          /* Elevator Seek Enable.    */
#define OVS            BIT7          /* Overlap Seek Enable.     */

#define ECC_M0         0             /* ECC Mode 0 */
#define ECC_M1         1             /* ECC Mode 1 */
#define ECC_M2         2             /* ECC Mode 2 */
   
/* CONTROL/STATUS REGISTER. */

#define CLRBS    BIT0    /* (W) Clear Register Busy Semaphore.  */ 
#define RBS      BIT0    /* (R) Register Busy Semaphore.        */
#define CLRIO    BIT1    /* (W) Clear Remove IOPB Bit.          */
#define RIO      BIT1    /* (R) Remove IOPB Status.             */
#define AIO      BIT2    /* (W) Add IOPB.                       */
#define AIOP     BIT2    /* (R) Add IOPB Pending Status.        */
#define RESET    BIT3    /* (W) Controller Reset.               */
#define RESETA   BIT3    /* (R) Controller Reset Active.        */
#define MM       BIT5    /* (W) Maintenance Mode Bit.           */
#define MMA      BIT5    /* (R) Maintenance Mode Active Status. */
#define FERR     BIT6    /* (R) Fatal Error Status.             */
#define CBUSY    BIT7    /* (R) Ctlr Busy Processng IOPB.       */

/* ADDRESS MODIFIER / PRIORITY IOPB REGISTER */

#define PRIORITY BIT7

/* FATAL ERROR REGISTER */

#define MAINT_MODE	0xe0    /* Maintenance Mode. */
#define SELF_1		0xe1    /* Power-Up Self Test Failed */	
#define SELF_2		0xe2    /* Power-Up Self Test Failed */	
#define SELF_3		0xe3    /* Power-Up Self Test Failed */	
#define SELF_4		0xe4    /* Power-Up Self Test Failed */	
#define SELF_5		0xe5    /* Power-Up Self Test Failed */	
#define SELF_6		0xe6    /* Power-Up Self Test Failed */	
#define SELF_7		0xe7    /* Power-Up Self Test Failed */	
#define SELF_8		0xe8    /* Power-Up Self Test Failed */	
#define SELF_9		0xe9    /* Power-Up Self Test Failed */	
#define CSMIS		0xf0    /* Checksum Miscompare.      */
#define DMAFATAL        0xf1    /* DMA Fatal.                */
#define BADIOPB         0xf2    /* IOPB Adress Error.        */ 
#define FW_ERROR        0xf3    /* Firmware Error.           */ 

/* Controller Specific Status */

        /* BYTE 1 */

#define SG_Align_Err             0x1f  /* Scatter/Gather List Alignment Error */
#define SG_ECC_Err               0x20  /* Scatter/Gather w/Auto ECC Error */
#define Interface_Err            0x4c
#define Attention_Hi             0x65
#define Cmd_Not_Complete         0x66
#define Drv_Interface_Flt        0x67
#define Wt_Gate_Trk_Flt          0x68
#define Ill_Drv_Cmd              0x69
#define Vendor_Status            0x6a
#define Cmd_Parity_Flt           0x6b
#define Power_Fault              0x6c

        /* BYTE 2 */

#define CMPL           BIT1      /* Command Complete */
#define ATTN           BIT3      /* Drive Reporting Interface Error */

        /* BYTE 6 */

#define SSF            BIT5       /* Soft Sectored Format. */

        /* BYTE 8 */

#define EDT            BIT3          /* Enable DMA Timeout.           */

        /* BYTE A */

#define IEC            BIT5          /* Interrupt At End Of Chain*/


/******************************************************************************

DESCRIPTION:
		This generic header file contains the structure 
                definitions for the partition, unit, controller, ioctl and 
                format information tables.
                                                                            
*******************************************************************************/

/* The driver's idea of an IOPB. */

typedef struct
{
    UCHAR    byte[ISIZE];     /* Actual IOPB sent to controller.       */
    BUFPTR   buffer;          /* Pointer to associated buf structure.  */
    int      mb_address;      /* Memory Map value from mbsetup()-(SUN) */
    UINT     flags;
} IOPB, *IOPBPTR;
#define IOPBACTIVE BIT0;	/* command in progress flag */

/* VME Bus Controller Register Declaration */

typedef struct
{
    unsigned char fill1;   
    unsigned char addr1;   /* Real High Address Byte */
    unsigned char fill2;   
    unsigned char addr2;   /* High Address Byte */
    unsigned char fill3;   
    unsigned char addr3;   /* Low Address Byte */
    unsigned char fill4;   
    unsigned char addr4;   /* Real Low Address Byte */
    unsigned char fill5;   
    unsigned char modifier;/* Address Modifier Byte */
    unsigned char fill6;  
    unsigned char csr;     /* Status Reg if Read, Command Reg if Written */
    unsigned char fill7;   
    unsigned char fatal;   /* Fatal Error Code Register */
} REGISTERS;

#define ADDR_1   0
#define ADDR_2   1
#define ADDR_3   2
#define ADDR_4   3
#define MODIFIER 4
#define CSR      5
#define FATAL    6


/* Unit Information Table. */

typedef struct 
{
    UCHAR              u_status;
    USHORT             heads;
    USHORT             cylinders;
    USHORT             sectors;   
    USHORT             sect_size;
    USHORT             trk_spares;
    USHORT             cyl_spares;
    USHORT             alt_tracks;
    UCHAR              interleave;
    UCHAR              int_level;
    UCHAR              vector;
    UCHAR              spare1;
#define drive_parms spare1
    UCHAR              spare2;
    UCHAR              spare3;
    UCHAR              spare4;
    PARTINFO          *part_table;
    REGISTERS         *reg_address;
    char 	      *spare5;		/* previously u_name */
    char              *bad_block;
    char              *u_future;
} UNITINFO;


/* Controller Information Table. */

typedef struct 
{
    UCHAR              c_status;
    UCHAR              param1;
    UCHAR              param2;
    UCHAR              param3;
    UCHAR              throttle;
    REGISTERS         *reg_address;
    UNITINFO          *units;
    char              *c_name; 
    char              *c_future;
} CTLRINFO;

/* Format Information Table. */

typedef struct 
{
    UCHAR              interleave;
    UCHAR              field_1;
    UCHAR              field_2;
    UCHAR              field_3;
    UCHAR              field_4;
    UCHAR              field_5H;
    UCHAR              field_5L;
    UCHAR              field_5AH;
    UCHAR              field_5AL;
    UCHAR              field_6;
    UCHAR              field_7;
    UCHAR              field_12;
} FORMATINFO;


/* Ioctl Table. */

typedef struct 
{
    UCHAR         status;
    USHORT        var_1;
    USHORT        var_2;
    USHORT        var_3;
    ULONG         long_var;
    char         *ptr_1;
    char         *ptr_2;
    char         *ptr_3;
} IOCTL_TABLE;


/* Format Information Table. */

typedef struct 
{
    UCHAR         f_status;
    USHORT        track;
    USHORT        head;
    USHORT        cylinder;
    USHORT        sector;
} FORMAT_TABLE;

/* Queue Structure Declaration */

typedef  struct 
{
    UCHAR      q_status;   /* Queue status.                        */
    IOPBPTR    ready;      /* Pointer to head of READY IOPB Queue. */
    IOPBPTR    free;       /* Pointer to head of FREE IOPB Queue.  */
    BUFPTR     buffs;      /* Pointer to head of BUF Queue.        */
} QUEUE;


