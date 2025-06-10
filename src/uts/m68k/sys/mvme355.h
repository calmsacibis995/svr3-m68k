/*      itreg.h 0.0     86/02/10        Integrated Solutions - RLW */
/*
 * Structure of IO Parameter Block (IOPB)
 */

typedef unsigned long  u_long;
typedef unsigned int   u_int;
typedef unsigned short u_short;
typedef unsigned char  u_char;

struct iopb {				/* IOPB - I/O Parameter Block       */
        u_char  pb_cmd;		/* command register                 */
#define		C_DIAG		0x70    /* diagnostic                       */
#define		C_XRPT		0x76    /* report extended drive status     */
#define		C_CFGRPT	0x77    /* report configuration             */
#define		C_WRTBUF	0x78    /* write data buffer                */
#define		C_RDBUF		0x79    /* read data buffer                 */
#define		C_OPTTAP	0x7A    /* issue optional tape command      */
#define		C_OPSTAT	0x7B    /* read optional tape status        */
#define		C_RDBLK		0x81    /* read n block(s) forward          */
#define		C_WRTBLK	0x82    /* write n block(s)                 */
#define		C_VERBLK	0x83    /* verify n block(s) forward        */
#define		C_WRTFMK	0x84    /* write n filemarks                */
#define		C_EDWRT		0x85    /* edit write                       */
#define		C_HSHK 		0x86    /* handshake                        */
#define		C_INIT		0x87    /* initialize                       */
#define		C_REWIND	0x89    /* rewind (to load point)           */
#define		C_REWQCK	0x8A    /* rewind quick                     */
#define		C_WBKFMK	0x8B    /* write n blocks and N filemarks   */
#define		C_MFWDBK	0x8D    /* move forward n blocks            */
#define		C_MBWDBK	0x8E    /* move backware n blocks           */
#define		C_MFWDFM	0x91    /* move forward n filemarks         */
#define		C_MBWDFM	0x92    /* move backward n filemarks        */
#define		C_MFWDET	0x93    /* move forward to eot              */
#define		C_35ERS		0x95    /* erase 3.5 inches                 */
#define		C_OFFLN		0x96    /* set drive offline                */
#define		C_ONLINE	0x97    /* set drive online                 */
#define		C_RSTFMT	0x9A    /* reset formatter                  */
#define		C_FETCHX	0x9B    /* fetch & execute iopb             */
#define		C_ERSEOT	0x9C    /* erase to eot                     */
#define		C_VRLNER	0x9D    /* variable length erase            */
#define		C_VREVBK	0x9E    /* verify n block(s) reverse        */
#define		C_RDRVBK	0x9F    /* read n block(s) reverse          */
        
	u_char  pb_coptn;	/* command code options             */
#define         CO_RTHRN	0x80    /*    read threshhold enable     @  */
#define         CO_DEFN		0x40    /*    defer enable               @  */
#define         CO_RES 		0x20    /*    reserved - must be 0          */
#define         CO_EDEDS	0x10    /*    error detection disable       */
#define         CO_INTEN	0x08    /*    interrupt enable              */
#define         CO_UN_B2	0x04    /*    tape unit number bit 2        */
#define         CO_UN_B1	0x02    /*    tape unit number bit 1        */
#define         CO_UN_B0	0x01    /*    tape unit number bit 0        */
/*   - @ -         not implemented in beta site release                     */
	u_char  pb_stat;	/* status code register             */
#define         ST_NO_ER	0x80    /*    completed with no errors      */
#define         ST_BUSY		0x81    /*    command in progress           */
#define         ST_CP_ER	0x82    /*    completed with error          */
#define         ST_CP_EX 	0x83    /*    completed with exception      */
        
	u_char  pb_er_cd;	/* error code register              */
#define         ER_NRDY		0x10    /* tape not ready                   */
#define         ER_IVCD		0x14    /* invalid command code             */
#define         ER_ILFTC	0x15    /* illegal fetch & execute command  */
#define         ER_ILMTP	0x17    /* illegal memory type              */
#define         ER_BUS_T	0x18    /* bus time out                     */
#define         ER_WRTPR	0x1A    /* tape write protected             */
#define         ER_EOT  	0x20    /* end of tape                      */
#define         ER_LDPNT	0x21    /* load point error                 */
#define         ER_UNCRT	0x23    /* uncorrectable error              */
#define		ER_RD_LG	0x25	/* record longer than specified	    */
#define		ER_RD_ST	0x26	/* record shorter than specified    */
#define         ER_SGDEF	0x42    /* signal definition error          */
#define         ER_BUSER	0x52    /* VME bus error                    */
#define         ER_FIFO 	0x58    /* FIFO error                       */
#define         ER_IVMOD	0x5A    /* invalid address modifiers        */
#define         ER_IVADR	0x5B    /* invalid memory address           */
#define         ER_INTLV	0x5C    /* invalid interrupt level          */
#define         ER_BURST	0x5D    /* invalid dma burst count          */
#define         ER_IOPBF	0x60    /* iopb failed                      */
#define         ER_DMAF 	0x61    /* DMA failed                       */
#define         ER_IVUIB	0x62    /* invalid UIB parameter            */
#define         ER_TPETM	0x63    /* tape timeout                     */
#define         ER_IIOPB	0x64    /* invalid IOPB parameter           */
#define         ER_IRECS	0x65    /* invalid record size              */
#define		ER_XF_SZ	0x66	/* invalid transfer size            */
#define		ER_ER_SZ	0x67	/* invalid transfer size            */
#define         ER_FILEMK	0x68    /* unexpected filemark	            */
#define         ER_CL_CP	0x69    /* error in close coupling	    */
#define         ER_RWDTM	0x70    /* time out on rewind               */
#define         ER_RE_WD	0x71    /* rewind not accepted              */
#define         ER_CMD_A	0x80    /* command aborted                  */
#define         ER_N_IMP	0xFF    /* command not implemented          */

        u_short pb_hb_rec;	/* high word bytes per record       */
        u_short pb_lb_rec;	/* low word bytes per record        */
        u_short pb_rec_c;	/* record count                     */
        u_short pb_hbuf_addr;	/* high word buffer address         */
        u_short pb_lbuf_addr;	/* low word buffer address          */
        u_char  pb_m_type;	/* memory type                      */
#define 	MT_8BIT		0x00    /*    8 bit memory  (not used)      */
#define 	MT_I16BIT	0x01    /*    16 bit internal memory        */
#define 	MT_16BIT	0x02    /*    16 bit memory                 */
#define 	MT_32BIT	0x03    /*    32 bit memory                 */

        u_char  pb_adr_mod;	/* address modifier code            */
#define 	AM_SD_SP	0x3E    /*    Standard Supervisory Program  */
#define 	AM_SD_SD	0x3D    /*    Standard Supervisory Data     */
#define 	AM_SD_NP	0x3A    /*    Standard Normal Program       */
#define 	AM_SD_ND	0x39    /*    Standard Normal Data          */
#define 	AM_ST_S		0x2D    /*    Short Supervisory IO          */
#define 	AM_ST_N		0x29    /*    Short Normal IO               */
#define 	AM_EX_SP	0x0E    /*    Extended Supervisory Program  */
#define		AM_EX_SD	0x0D    /*    Extended Supervisory Data     */
#define		AM_EX_NP	0x0A    /*    Extended Normal Program       */
#define		AM_EX_ND	0x09    /*    Extended Normal Data          */

        u_char  pb_int_level;		/*    interrupt level               */
        u_char  pb_dma_burst;		/*    DMA  burst count              */
#define 	DMA_BURST	8	/*    8 vme cycles per transfer     */

        u_char  pb_n_vec;          /* normal comp. interrupt vector    */
        u_char  pb_e_vec;          /* error Interrupt vector           */
        u_char  pb_fm_cnt;         /* filemark count                   */
        u_char  pb_rv1;		/* reserved                         */
        u_short pb_hnotdma;	/* high word not dma'd              */
        u_short pb_lnotdma;	/* low word not dma'd               */
        u_short pb_notrec;	/* record count not transfered      */
        u_short pb_rv2;		/* reserved                         */

    };

/*
 * Structure for a command pair (csr/iopb)
 */
struct command {
        u_short cstatus;
#define         CS_SLED         0x8000  /*    Status LED                    */
#define         CS_BOK          0x4000  /*    board ok                      */
#define         CS_SFEN         0x2000  /*    sysfail enable                */
#define         CS_BDCLR        0x1000  /*    board clear                   */
#define         CS_ABORT        0x0800  /*    abort                         */
#define         CS_RES1         0x0400  /*    reserved                      */
#define         CS_RES2         0x0200  /*    reserved                      */
#define         CS_BERR         0x0100  /*    bus error                     */
#define         CS_GO           0x0080  /*    execute command               */
#define         CS_BUSY         0x0080  /*    executing command             */
#define         CS_DONE         0x0040  /*    operation complete (interrupt)*/
#define         CS_RES3         0x0020  /*    reserved                      */
#define         CS_ER_LC	0x0010  /*    error on last command         */
#define         CS_RES4         0x0008  /*    reserved                      */
#define         CS_RES5         0x0004  /*    reserved                      */
#define         CS_RES6         0x0002  /*    reserved                      */
#define         CS_RES7         0x0001  /*    reserved                      */
        struct iopb  pb;
};

/*
 * Structure of Unit Initialization Block
 */
struct uib {                    /* Unit Initialization Block        */
        u_char  uib_J114;	/* J1-14 select                     */
        u_char  uib_J116;	/* J1-16 select                     */
        u_char  uib_J136;	/* J1-36 select                     */
        u_char  uib_J144;	/* J1-44 select                     */
        u_char  uib_J226;	/* J2-26 select                     */
        u_char  uib_J250;	/* J2-50 select                     */
        u_char  uib_rsv1;	/* reserved (must be 0)             */
        u_char  uib_rsv2;	/* reserved (must be 0)             */
        u_char  uib_rsv3;	/* reserved (must be 0)             */
        u_char  uib_rsv4;	/* reserved (must be 0)             */
        u_char  uib_rsv5;	/* reserved (must be 0)             */
        u_char  uib_retry;	/* retry count                      */
        u_char  uib_rsv6; 	/* reserved (must be 0)             */
        u_char  uib_rsv7; 	/* reserved (must be 0)             */
        u_char  uib_attr;	/* attribute flags                  */
#define         ATT_MODE        0x80   /*     mode bit                     */
#define         ATT_RSV         0x40   /*     reserved                     */
#define         ATT_ERSEN       0x20   /*     erase enable                 */
#define         ATT_STC         0x10   /*     status change                */
#define         ATT_CONT        0x08   /*     continuous bit               */
#define         ATT_SWAP        0x04   /*     byte swap                    */
#define         ATT_ZERO        0x02   /*     must be 0                    */
#define         ATT_RTRY        0x01   /*     retry                        */

        u_char  uib_fill;	/* % of buffer fill/tape restart  @ */
        u_char  uib_lv_rw;	/* status change lev. for rew compl.*/
        u_char  uib_vc_rw;	/* status change vec. for rew compl.*/
        u_char  uib_lev;	/* status change interrupt level    */
        u_char  uib_vec;	/* status change interrupt vector   */
};

/*
 * tape unit status register definitions
 */
#define         SR_RDY		0x8000 /* drive ready                      */
#define         SR_ONLN		0x4000 /* on line                          */
#define         SR_FLPRO	0x2000 /* file protect                     */
#define         SR_REWD 	0x1000 /* rewind                           */
#define         SR_BOT  	0x0800 /* beginning of tape                */
#define         SR_EOT  	0x0400 /* end of tape                      */
#define         SR_FMK  	0x0200 /* filemark                         */
#define         SR_ID_B 	0x0100 /* ID burst                         */
#define         SR_RSV1 	0x0080 /* reserved                         */
#define         SR_SPD  	0x0040 /* speed                            */
#define         SR_GCR  	0x0020 /* GCR                              */
#define         SR_U_CK 	0x0010 /* unit check                       */
#define         SR_RSV2 	0x0008 /* reserved                         */
#define         SR_HARD 	0x0004 /* hard error                       */
#define         SR_COR_E	0x0002 /* correctable error                */
#define         SR_P_ER 	0x0001 /* parity error                     */

/*
 * Status change register definitions          
 */
#define         SCR_RSV1       0x8000 /* reserved                         */
#define         SCR_RSV2       0x4000 /* reserved                         */
#define         SCR_RSV3       0x2000 /* reserved                         */
#define         SCR_RSV4       0x1000 /* reserved                         */
#define         SCR_RSV5       0x0800 /* reserved                         */
#define         SCR_RSV6       0x0400 /* reserved                         */
#define         SCR_RSV7       0x0200 /* reserved                         */
#define         SCR_RSV8       0x0100 /* reserved                         */
#define         SCR_SCI        0x0080 /* status change interrupt          */
#define         SCR_RSV9       0x0040 /* reserved                         */
#define         SCR_RSVA       0x0020 /* reserved                         */
#define         SCR_RSVB       0x0010 /* reserved                         */
#define         SCR_RSVC       0x0008 /* reserved                         */
#define         SCR_FMT        0x0004 /* formatter number                 */
#define         SCR_UN_1       0x0002 /* unit number bit 1                */
#define         SCR_UN_0       0x0001 /* unit number bit 0                */
#define         SCR_UNMSK      0x0007 /* unit mask 			  */


/*
 * Structure of VTAPE short IO space
 */
struct vtape_shio
{
  struct command  vtp_cmd[7];
  u_short         vtp_sreg[8];
  u_short         vtp_screg;
  u_short         vtp_head;
  u_short         vtp_tail;
  u_char          vtp_slop[10];		/* slop space */
  u_char          vtp_xfer[0x100];
};

/*
 * tpioctl ----
 *      include file for tp.c tputil.c
 *      reminds one of mt stuff on 4.2
 */

#define	MTREW		1
#define	MTWEOF		2
#define	MTFSF		3
#define	MTBSF		4
#define	MTFSR		5
#define	MTBSR		6
#define	MTOFFL		7
#define	MTNOP		8
#define	MTERASE		9
#define	MTIOCGET	10
#define	IOMESS		11
#define	IOIPB		12
#define IOUIB		13
#define IOUNLOCK	14
#define IOSTAT		15
#define IOINIT		16

