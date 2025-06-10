/*	@(#)mvme320.h	6.3		*/
/*
  start of mvme320 header file m320.h
*/

/* internal typedefs */

typedef char                     byte;
typedef unsigned char            ubyte;
typedef short int                int16;
typedef unsigned short int       uint16;
typedef int                      int32;
typedef unsigned int             uint32;

/* control and status registers */

struct m3csr
{
                                 /* interface is byte-wide, no even regs */
  ubyte   reg0;
  ubyte   reg1;                  /* eca ptr-table ptr byte 0 (low) */
  ubyte   reg2;
  ubyte   reg3;                  /* ... byte 1 */
  ubyte   reg4;
  ubyte   reg5;                  /* ... byte 2 */
  ubyte   reg6;
  ubyte   reg7;                  /* ... byte 3 */
  ubyte   reg8;
  ubyte   reg9;                  /* interrupt vector */
  ubyte   regA;
  ubyte   regB;                  /* interrupt source mask */
  ubyte   regC;
  ubyte   regD;                  /* busy mask */
  ubyte   regE;
  ubyte   regF;                  /* not used */
};

/* event control area (command/status communications block) */

struct m3e_eca
{
  ubyte   m3e_cmd;               /* command code */
  ubyte   m3e_mnst;              /* main status */
  uint16  m3e_exst;              /* extended status */
  ubyte   m3e_mrty;              /* max retries */
  ubyte   m3e_arty;              /* actual retries */
  ubyte   m3e_dma;               /* dma type */
  ubyte   m3e_copt;              /* command options */
  uint32  m3e_bufa;              /* buffer address */
  uint16  m3e_bufl;              /* buffer length (requested) */
  uint16  m3e_actr;              /* actual transfer count */
  uint16  m3e_cyl;               /* cylinder */
  ubyte   m3e_head;              /* head (surface) */
  ubyte   m3e_sect;              /* sector */
  uint16  m3e_curc;              /* current cylinder */
  uint16  m3e_rsv1 [5];          /* reserved */
  ubyte   m3e_n0;                /* pre-index gap */
  ubyte   m3e_n1;                /* post-index gap */
  ubyte   m3e_n2;                /* sync byte count */
  ubyte   m3e_n3;                /* post-id gap */
  ubyte   m3e_n4;                /* post-data gap */
  ubyte   m3e_n5;                /* address-mark count */
  ubyte   m3e_slc;               /* sector length code */
  ubyte   m3e_fill;              /* fill byte */
  uint16  m3e_rsv2 [3];          /* reserved */
  ubyte   m3e_drvt;              /* drive type */
  ubyte   m3e_nhed;              /* number of heads (surfaces) */
  ubyte   m3e_nspt;              /* number of sectors per track */
  ubyte   m3e_step;              /* stepping rate */
  ubyte   m3e_setl;              /* head settling time */
  ubyte   m3e_load;              /* head load time */
  ubyte   m3e_seek;              /* seek type */
  ubyte   m3e_phas;              /* "phase count" - firmware state variable */
  uint16  m3e_lowr;              /* low write-current track (cyl) */
  uint16  m3e_pcmp;              /* precompensation track */
  uint16  m3e_eccr [3];          /* ecc remainder */
  uint16  m3e_ecca [3];          /* append ecc remainder */
  uint16  m3e_rsv4 [2];          /* reserved */
  uint16  m3e_320w [6];          /* mvme320 working area */
  uint16  m3e_rsv5;              /* reserved */
};

/* commands */

#define   M3C_CALB       0       /* recalibrate */
#define   M3C_WRDD       1       /* write deleted data */
#define   M3C_VERF       2       /* verify (read without transfer) */
#define   M3C_TSRD       3       /* transparent sector read */
#define   M3C_REMS       5       /* read multiple sectors with seek */
#define   M3C_WRMS       6       /* write multiple sectors with seek */
#define   M3C_FORM       7       /* format a track */

/* main status codes */

#define   M3S_OK         0       /* no error */
#define   M3S_NONR       1       /* non-recoverable after retries */
#define   M3S_NRDY       2       /* drive not ready */
#define   M3S_SECA       4       /* sector address out of range */
#define   M3S_TPUT       5       /* throughput error (overrun) */
#define   M3S_RJCT       6       /* command rejected */
#define   M3S_BUSY       7       /* controller busy */
#define   M3S_DRNA       8       /* drive not available (head out of range) */
#define   M3S_DMAC       9       /* DMA bus error */
#define   M3S_ABRT      10       /* command abort */

/* extended error status bits */

#define   M3X_WFLT       0x0001  /* write fault */
#define   M3X_CECC       0x0002  /* CRC or ECC error (id or data) */
#define   M3X_OVRN       0x0004  /* data overrun */
#define   M3X_NOID       0x0008  /* no id found */
#define   M3X_NRDY       0x0010  /* not ready */
#define   M3X_DELT       0x0020  /* deleted data address mark */
#define   M3X_PROT       0x0040  /* write on protected diskette */
#define   M3X_POSE       0x0080  /* positioning error */
#define   M3X_DTIM       0x0100  /* data port timeout */
#define   M3X_FORM       0x0200  /* disk format error */
#define   M3X_UCOR       0x0400  /* uncorrectable data error (ECC) */
#define   M3X_DRNA       0x0800  /* drive not available and command stop */
#define   M3X_DTYP       0x1000  /* drive type rejected */
#define   M3X_PTIM       0x2000  /* positioning timeout */
#define   M3X_RTRK       0x4000  /* wrong id-data-id sequence during read track */
#define   M3X_BERR       0x8000  /* bus error fault */


/* parameters */

#define MAXCTL     2
#define MAXUNIT    4
#define M320SLICE   8		/* maximum number of slices */
#define MAXXFER    64512  /* note: 63K since 0 => seek-only */

/* macros */

#define CTL(x)     ((x >> 6) & 1)
#define UNIT(x)    ((x >> 3) & 3)
#define SLICE(x)   (x & 7)
#define ILLEGAL(x) (x & 0xA0)
#define FLOPPY(x)  (UNIT(x) > 1)
#define MKDEV(x,y,z)  (x << 6 | y << 3 | z)

struct  m320eregs {
/*  0 */ unsigned char	er_mnst;	/* main status */
/*  1 */ unsigned char	er_exst;	/* extended status */
/*  2 */ ushort		er_bufl;	/* requested data transfer */
/*  4 */ ushort         er_actr;        /* actual data transfer */
/*  6 */ unsigned char	er_eccr[3];	/* ecc remainder */
/*  7 */ unsigned char	er_cmd;		/* command */
};

#define	M320EREG	(sizeof(struct m320eregs)/sizeof(short))

/* end of header file m320.h */
