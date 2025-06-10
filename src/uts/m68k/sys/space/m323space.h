/*	@(#)m323space.h	7.5 	*/
/*
 *	Copyright (c) 1987	Motorola Inc.  
 *		All Rights Reserved.
 */
/*
 *	This file defines external definitions for
 *	the MVME323 Storage Module Disk Controller.
 */


#include "sys/mvme323.h"	/* m323config */

struct	m323config	m323config[M323CTLS*M323DRV];
struct	m323eregs	m323eregs[M323CTLS];

/* 0 in the following define should be M323 major number */
#define	m323sa(x)	tabinit(6,&m323stat[x].ios)

struct	iotime		m323stat[M323CTLS*M323DRV];
struct	iobuf		m323utab[M323CTLS*M323DRV] = {
	m323sa(0), m323sa(1), 
	m323sa(2), m323sa(3),
};

#undef	m323sa

/*
 *	M323 Disk type support
 */

#define	M323H182	1	/* CDC WREN 3 182 Mbyte ESDI Disk Drive */

#define	M323TYPES	1

/*
 *	default configuration table ... indexed by type
 */


/* THIS TABLE IS NOT USED CURRENTLY, BUT WILL BE USED AT A FUTURE DATE. */
unsigned char m323types[M323CTLS*M323DRV] = {
	M323H182, M323H182, M323H182, M323H182,
};

#include "sys/io/esdi.h"
PARTINFO part_table[M323DRV * M323SLICE] =
{
	CDC182MB	/* Unit 0  CDC 182 MB Drive */
	CDC182MB	/* Unit 1  CDC 182 MB Drive */
	CDC182MB	/* Unit 2  CDC 182 MB Drive */
	CDC182MB	/* Unit 3  CDC 182 MB Drive */
};


/* The following should be in a per controller structure if  
 multiple controller operation is ever supported. */
IOPBPTR iopb[M323ICNT];             /* Array of allocated IOPB pointers.    */
IOPBPTR riopb[M323DRV];            /* Array of raw IOPB pointers.          */
IOPB    io_vec[M323ICNT + M323DRV + 1]; /* Vector of IOPB Structures       */
BUF     rbuf[M323DRV];             /* Array of raw buf structures.         */
short	m323qcnt;	/* count of IOPBs in the controllers queue */
short   m323rcnt;	/* count of IOPBs sent to controller since the 
			   controllers queue was last empty. */
char	m323cflush;	/* flag used by driver to withhold IOPBs from the 
			   controller to allow controller to complete all 
			   IOPBs in its queue. */
				


#include "sys/open.h"
int m323otbl[M323CTLS * M323DRV * OTYPCNT]; /* Array of driver open tables    r.k. */


/* THIS TABLE IS NOT USED CURRENTLY, BUT WILL BE USED AT A FUTURE DATE. */
struct m323config m323dfcf[M323TYPES] = {
   9,		/* number of heads */
   35,		/* sectors per track */
   512,		/* bytes per sector */
   968,		/* number of cylinders */
   1,		/* interleave factor */
   0,		/*  bogus gap 1 */
   0,		/* bogus gap 2 */
};

UNITINFO unit_table[M323DRV] =
{
    0,                 /* Status                  */
    9,                 /* Heads                   */
    968,               /* Cylinders               */
    35,                /* Sectors                 */
    512,               /* Sector Size             */
    1,                 /* Track Spares            */
    0,                 /* Cylinder Spares         */
    144,               /* Alternate Tracks        */
    1,                 /* Interleave              */
    0,                 /* Interrupt Level         */
    0,                 /* Vector                  */
    EC32,              /* Drive Parameter Byte 06 */
    0,                 /* Spare2                  */
    0,                 /* Spare3                  */
    0,                 /* Spare4                  */
    0, /* &part_table[0][0],*/  /* Partition Table Pointer */
    0,                 /* Ctlr Registers Pointer   */
    0,			/* Spare5 previously u_name */
    0,                 /* Bad Block Pointer       */
    0,                 /* Future Use Pointer      */

    0,                 /* Status                  */
    9,                 /* Heads                   */
    968,               /* Cylinders               */
    35,                /* Sectors                 */
    512,               /* Sector Size             */
    1,                 /* Track Spares            */
    0,                 /* Cylinder Spares         */
    144,               /* Alternate Tracks        */
    1,                 /* Interleave              */
    0,                 /* Interrupt Level         */
    0,                 /* Vector                  */
    EC32,              /* Drive Parameter Byte 06 */
    0,                 /* Spare2                  */
    0,                 /* Spare3                  */
    0,                 /* Spare4                  */
    0, /* &part_table[1][0], */  /* Partition Table Pointer */
    0,                 /* Ctlr Registers Pointer   */
    0,			/* Spare5 previously u_name */
    0,                 /* Bad Block Pointer       */
    0,                 /* Future Use Pointer      */

    0,                 /* Status                  */
    9,                 /* Heads                   */
    968,               /* Cylinders               */
    35,                /* Sectors                 */
    512,               /* Sector Size             */
    1,                 /* Track Spares            */
    0,                 /* Cylinder Spares         */
    144,               /* Alternate Tracks        */
    1,                 /* Interleave              */
    0,                 /* Interrupt Level         */
    0,                 /* Vector                  */
    EC32,              /* Drive Parameter Byte 06 */
    0,                 /* Spare2                  */
    0,                 /* Spare3                  */
    0,                 /* Spare4                  */
    0, /* &part_table[2][0], */  /* Partition Table Pointer */
    0,                 /* Ctlr Registers Pointer   */
    0,			/* Spare5 previously u_name */
    0,                 /* Bad Block Pointer       */
    0,                 /* Future Use Pointer      */

    0,                 /* Status                  */
    9,                 /* Heads                   */
    968,               /* Cylinders               */
    35,                /* Sectors                 */
    512,               /* Sector Size             */
    1,                 /* Track Spares            */
    0,                 /* Cylinder Spares         */
    144,               /* Alternate Tracks        */
    1,                 /* Interleave              */
    0,                 /* Interrupt Level         */
    0,                 /* Vector                  */
    EC32,              /* Drive Parameter Byte 06 */
    0,                 /* Spare2                  */
    0,                 /* Spare3                  */
    0,                 /* Spare4                  */
    0, /* &part_table[3][0], */ /* Partition Table Pointer */
    0,                 /* Ctlr Registers Pointer  */
    0,			/* Spare5 previously u_name */
    0,                 /* Bad Block Pointer       */
    0,                 /* Future Use Pointer      */
};

/* WARNING: These format parameters must be compatible with all drives used 
            by the controller.
 */
FORMATINFO format_parm =
{
	1,		/* interleave */
	12,		/* field 1 - Read Gate Delay - Gap 1 */
	5,		/* field 2 - Sync Search Delay */
	23,		/* field 3 - Header Preamble */
	12,		/* field 4 - Gap 2 - time between the end of the */
			/*    header ECC Pad and the Data Sync Byte. */
	0x02,		/* sector size ms byte */
	0x00,		/* sector size ls byte */
	0x02,		/* alternate sector size ms byte - not used */
	0x00,		/* alternate sector size ls byte - not used */
	5,		/* field 6 - Search Gate Delay - during Gap 2 */
	2,		/* field 7 - Write continuation */
	10,		/* field 12 - Head switch time */
};

CTLRINFO ctlr_table = 
{
         0,                       /* Status                      */
         TMOD|EDT,                /* Parameter Byte 08 (32 bit xfer) */
         ROR,                     /* Param Byte 09 (0 us throttle deadtime */
         OVS|COP|ASR|ZLR|RBC|ECC_M2, /* Parameter Byte 0A         */
         0x10,                    /* Parameter Byte 0B (16 byte Throttle) */
         0,                       /* Ctlr Register Pointer       */
         0, /*  &unit_table[0], */ /* Unit Info Table Pointer     */
         0,                       /* Controller Name             */
         0,                       /* Future Use Pointer          */
 };


