/*	@(#)mvme360.h	7.5 	*/
/*
 *	Copyright (c) 1985	Motorola Inc.  
 *		All Rights Reserved.
 */
/*
 *	mvme360.h: include file for Motorola MVME360 disk controller
 */

/*
 *	global defines
 */

#ifdef	MVME360
#define	BADBLK	1			/* at location 512 bytes into disk */
#endif

#define M360NLCF	0xff		/* null configuration magic number */

/*
 *	ioctl functions
 */

#define	M360FMTT	0	/* format track */
#define	M360GET		1	/* get configuration */
#define	M360SET		2	/* set configuration */
#define	M360RST		3	/* restore drive */
#define	M360CLRF	4	/* clear fault */
#define	M360VRFY	5	/* verify track */
#define	M360COFF	6	/* cache off */
#define	M360CON		7	/* cache on */
#define M360MPT		10	/* map alternate track */
#define M360MPS		11	/* map track with sector slip */
#define	M360RFMT	12	/* reformat track saving alternates */

/*
 *	On the ioctl function,
 *
 *	M360FMTT: requires that arg be a pointer to a struct m360ctl.
 *	M360GET: requires that arg be a pointer to a struct config.
 *	M360SET: requires that arg be a pointer to a struct config.
 *	M360MTP: requires that arg be a pointer to a struct m360ctl.
 */

typedef struct {
	int  f_trk;	/* Track number */
	int  f_alttrk;	/* Alternate track number */
	int  f_blk;	/* Beginning block number of the track */
	int  f_altblk;	/* Beginning block number of the alternate track */
	int  f_slpsec;	/* Sector to be slipped	*/
} m360fmt;



/*
 *	Configuration Control: Only needs certain fields from dkconfig
 */

struct m360config {
/*  0 */ unsigned char	cf_vsh;		/* volume starting head */
/*  1 */ unsigned char	cf_vnh;		/* volume number of heads */
/*  2 */ unsigned char	cf_spt;		/* sectors per track */
/*  3 */ unsigned char	cf_skew;	/* spiral skewing factor */
/*  4 */ ushort		cf_bps;		/* bytes per sector */
/*  6 */ unsigned char	cf_gap1;	/* words in gap 1 */
/*  7 */ unsigned char	cf_gap2;	/* words in gap 2 */
/*  8 */ unsigned char	cf_intlv;	/* sector interleave factor */
/*  9 */ unsigned char	cf_retry;	/* number of retries on data error */
/* 0a */ ushort		cf_cyls;	/* number of cylinders */
/* 0c */ ushort		cf_attrib;	/* attributes */
};

/*
 *	configuration attribute flags & masks
 */

/* driver selectable */
#define	M360RSK		0x01		/* restore & reseek after retry */
#define	M360MBD		0x02		/* move bad data into memory */
#define	M360INH		0x04		/* increment by head */
#define	M360DLP		0x08		/* dual port protocol enable */
#define	M360STC		0x10		/* interrupt on status change */
/* user selectable */
#define	M360CEN		0x20		/* sector caching enable */
#define	M360SSE		0x40		/* spare sector enable */

#define M360DATR (M360RSK|M360INH)	/* driver selected attributes */
#define M360UATR	0x60		/* user selectable attribute mask */


/*
 *	MVME360 Unit Initialization Block
 */

typedef struct {
	unsigned char	hd_strt;	/* starting head */
	unsigned char	hd_cnt;		/* number of heads */
} head_rec;

struct	m360uib {
/*  0 */ head_rec	ib_hd[2];	/* per volume head record */
/*  4 */ unsigned char	ib_spt;		/* sectors per track */
/*  5 */ unsigned char	ib_skew;	/* spiral skewing factor */
/*  6 */ ushort		ib_bps;		/* bytes per sector */
/*  8 */ unsigned char	ib_gap1;	/* words in gap 1 */
/*  9 */ unsigned char	ib_gap2;	/* words in gap 2 */
/* 0a */ unsigned char	ib_intlv;	/* sector interleave factor */
/* 0b */ unsigned char	ib_retry;	/* number of retries on data error */
/* 0c */ ushort		ib_cyls;	/* number of cylinders */
/* 0e */ unsigned char	ib_attrib;	/* attribute flags */
/* 0f */ unsigned char	ib_r0;		/* reserved */
/* 10 */ unsigned char	ib_silev;	/* status change interrupt level */
/* 11 */ unsigned char	ib_sivec;	/* status change interrupt vector */
};

#define ib_sh(x) ib_hd[x].hd_strt
#define ib_nh(x) ib_hd[x].hd_cnt


/*
 *	MVME360 I/O Parameter Block
 */

struct	m360iopb {
/*  0 */ unsigned char	pb_cmd;		/* command */
/*  1 */ unsigned char	pb_opt;		/* command option */
/*  2 */ unsigned char	pb_stat;	/* status code */
/*  3 */ unsigned char	pb_err;		/* error code */
/*  4 */ union {
		unsigned long	iop_lsect;	/* logical sector */
		struct {
			ushort	iop_cyl;		/* cylinder */
			unsigned char	iop_trk;	/* track */
			unsigned char	iop_psect;	/* sector */
		} pb_typ1;
	} pb_un0;
/*  8 */ ushort		pb_scnt;	/* sector count */
/*  a */ union {
		char *iop_badr;		/* buffer address */
		struct {
			unsigned char	iop_alttrk;	/* alternate track */
			unsigned char	iop_f0;		/* fill */
			ushort	iop_pat;		/* data field pattern */
		} pb_typ2;
	 } pb_un1;
/*  e */ unsigned char	pb_bmtyp;	/* buffer memory type */
/*  f */ unsigned char	pb_bamod;	/* buffer address modifier */
/* 10 */ unsigned char	pb_ilev;	/* interrupt level */
/* 11 */ unsigned char	pb_nivec;	/* normal interrupt vector */
/* 12 */ unsigned char	pb_dmac;	/* DMA count */
/* 13 */ unsigned char	pb_eivec;	/* error interrupt vector */
/* 14 */ struct m360iopb *pb_iopbp;	/* iopb pointer */
/* 18 */ unsigned char	pb_imtyp;	/* iopb memory type */
/* 19 */ unsigned char	pb_iamod;	/* iopb address modifier */
/* 1a */ unsigned char	pb_skew;	/* spiral skew */
/* 1b */ unsigned char	pb_entries;	/* entry count */
};

#ifdef	MVME360
#define pb_lsect	pb_un0.iop_lsect
#define pb_cyl		pb_un0.pb_typ1.iop_cyl
#define pb_altcyl	pb_scnt
#define pb_trk		pb_un0.pb_typ1.iop_trk
#define pb_psect	pb_un0.pb_typ1.iop_psect
#define	pb_badr		pb_un1.iop_badr
#define	pb_pat		pb_un1.pb_typ2.iop_pat
#define	pb_alttrk	pb_un1.pb_typ2.iop_alttrk
#endif


struct  m360list {
	 ushort	bcount;		/* byte count */
	 char	*addr;		/* address */
	 unsigned char memtype;	/* link bit, reserve bits, and mem type */
	 unsigned char   addrmod; /* address modifier */
};
#define M360SECSIZ 512		/* sector size */
#define M360MXCOAL M360SECSIZ/sizeof(struct m360list) /* max scatter/gather */


struct	m360ctlr {
/*  0 */ unsigned int	cc_flags;	/* controller flags */
/*  4 */ struct iobuf	*cc_actp;	/* active iobuf pointer */
	 unsigned int	rev;    	/* controller firmware revision */
};

struct m360eregs {
/*  0 */ unsigned int	er_blkoff;	/* block offset of error */
/*  4 */ unsigned char	er_ds[2];	/* drive status for two drives */
/*  6 */ ushort		er_csr;		/* control status register */
/*  8 */ unsigned char	er_cmd;		/* command */
/*  9 */ unsigned char	er_opt;		/* command option */
/* 10 */ unsigned char	er_stat;	/* status code */
/* 11 */ unsigned char	er_err;		/* error code */
};

#define d0erstat	er_ds[1]
#define d1erstat	er_ds[0]
#define M360EREG	(sizeof(struct m360eregs)/sizeof(short))
