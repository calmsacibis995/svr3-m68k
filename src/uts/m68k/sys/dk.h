/*	@(#)dk.h	7.4	*/
/*
 *	Copyright (c) 1985	Motorola Inc.  
 *		All Rights Reserved.
 */
/*
 *	dk.h: Descriptions of Motorola Disk Format Standard
 *
 *
 *	Standard Disk Layout: optional fields can be zero length.
 *		In that case, the following field will immediately follow
 *		the previous field.
 *
 *	Size 	Contents
 *	----	--------
 *	  1	Volume ID Block
 *	  1	Disk Configuration Block
 *	 BH	Bad Track Hash Table (optional)
 *	 BT	Bad Track Table (optional)
 *	 DD	Diagnostic Test Area Directory (optional)
 *	 BO	Bootloader (up to 16 Kbytes)
 *	 FI	Cylinder Fill
 *	 DT	Diagnostic Tracks (optional)
 *	  N	Usable Disk Area (including alternate tracks)
 *	 DT	Diagnostic Tracks (optional)
 *
 *	Where:
 *	------
 *	BH	= if included then 1 else 0.
 *	BT	= if included then round(((tpc*maxbad+1)*4)/bps) else 0.
 *	DD	= if included then 1 else 0.
 *	BO	= 16K/bps.
 *	FI	= round((3+DD+BT+BO)/spc)*spc-(3+DD+BT+BO).
 *	DT	= if included then spc else 0.
 *	 N	= (spc*cyls)-(2+DD+BT+BO+FI+2*DT).
 *	 A	= maxbad*tpc*spt
 *
 *	and:
 *	----
 *	bps	= bytes per sector (>= 256)
 *	spc	= sectors per cylinder (== tpc*spt)
 *	spt	= sectors per track
 *	tpc	= tracks per cylinder
 *	cyls	= cylinder per disk
 *	maxbad	= maximum number of (software correctable) bad tracks
 *			allowed per surface (recording head).
 */

/*
 *	Volume ID structure: on first sector of disk
 */

struct	volumeid {
	unsigned	vid_vol;	/* Volume ID */
	unsigned char	vid_f0[16];	/* Zero fill */
	unsigned	vid_oss;	/* Bootloader starting sector */
	unsigned short	vid_osl;	/* Bootloader sector length */
	unsigned	vid_ose;	/* Bootloader execution address */
	unsigned	vid_osa;	/* Bootloader load address */
	unsigned char	vid_f1[4];	/* Zero fill */
	unsigned char	vid_vd[20];	/* ASCII Volume description */
	unsigned char	vid_f2[6];	/* Zero fill */
	unsigned char	vid_dtp[64];	/* Diagnostic test pattern */
	unsigned	vid_dta;	/* Diagnostic test area sector */
	unsigned char	vid_f3[12];	/* Zero fill */
	unsigned	vid_cas;	/* Configuration area starting sector */
	unsigned char	vid_cal;	/* Configuration area sector length */
	unsigned char	vid_f4[1];	/* Zero fill */
	unsigned short	vid_btl;	/* Number of bad tracks */
	unsigned short	vid_typ;	/* Disk type */
	unsigned char	vid_strat;	/* s/w = -1,  h/w = 1  */
	unsigned char	vid_f5[1];	/* Zero fill */
	unsigned	vid_rfs;	/* root file sys. offset in K bytes */
	unsigned char	vid_f6[88];	/* Zero fill */
	unsigned char	vid_mac[8];	/* ASCII string "MOTOROLA" */
};

/*
 *	config block: second sector on disk
 */

struct dkconfig {
	unsigned char	ios_f0[4];	/* Zero fill */
	unsigned short	ios_atm;	/* Attributes Mask */
	unsigned char	ios_f1[2];	/* Zero fill */
	unsigned short	ios_atw;	/* Attributes Word */
	unsigned short	ios_rec;	/* Logical sector size */
	unsigned	ios_rsz;	/* Number of logical sectors */
	unsigned char	ios_f2[8];	/* Zero fill */
	unsigned char	ios_spt;	/* Sectors per track */
	unsigned char	ios_hds;	/* Heads */
	unsigned short	ios_trk;	/* Number of cylinders */
	unsigned char	ios_ilv;	/* Interleave factor of media */
	unsigned char   ios_sof;	/* Spiral offset of media */
	unsigned short	ios_psm;	/* Physical sector size */
	unsigned short	ios_shn;	/* Starting head number of drive */
	unsigned char	ios_f3[2];	/* Zero fill */
	unsigned short	ios_pcom;	/* Precompensation cylinder */
	unsigned char	ios_f4[1];	/* Zero fill */
	unsigned char	ios_sr;		/* Stepping rate */
	unsigned char	ios_f5[2];	/* Zero fill */
	unsigned char   ios_ecc;	/* ECC data burst length */
	unsigned 	ios_eatm;	/* Extended attributes mask */
	unsigned 	ios_eprm;	/* Extended paramaters mask */
	unsigned 	ios_eatw;	/* Extended attributes word */
	unsigned char	ios_gpb1;	/* Gap byte 1 */
	unsigned char	ios_gpb2;	/* Gap byte 2 */
	unsigned char	ios_gpb3;	/* Gap byte 3 */
	unsigned char	ios_gpb4;	/* Gap byte 4 */
	unsigned char	ios_ssc;	/* Sector slip count */
	unsigned char	ios_f6[195];	/* Zero fill - Keep Out. */
};

/*  ABOVE FIELDS SHOULD LOOK LIKE THIS.  MAYBE NEXT RELEASE  ....
	unsigned short   ios_ecc;	/* ECC data burst length /
	unsigned short	ios_eatm;	/* Extended attributes mask *
	unsigned short	ios_eprm;	/* Extended paramaters mask *
	unsigned short	ios_eatw;	/* Extended attributes word *
	unsigned char	ios_f6[200];	/* Zero fill - Keep Out. *

*/

/* Bit definitions for ios_atw */

#define	ATWAS	0x0400	/* Alternate Sectors? 1=yes, 0=no */
#define	ATWFS	0x0200	/* Floppy size (0=5 1/4, 1=8) */
#define	ATWPC	0x0100	/* 0=pre-write precomp, 1=post-read precomp */
#define	ATWSK	0x0080	/* Seek after head change? 0=no, 1=yes */
#define	ATWDD	0X0040	/* Track Density of Drive (0=single, 1=double) */
#define	ATWEN	0X0020	/* Encoding (0=FM, 1=MFM) */
#define	ATWDT	0X0010	/* 0=floppy, 1=hard */
#define	ATWSN	0X0008	/* Sector Numbering (0=Motorola, 1=IBM) */
#define	ATWDS	0x0004	/* No. of sides (1=double, 0=single) */
#define	ATWTD	0x0002	/* Track Density of diskette (1 for 5 1/4, 0 for 8) */
#define	ATWMF	0x0001	/* Data Density of medium (0=single, 1=double) */

/*
 *	diagnostic track support
 */

struct	diagent {
	unsigned	d_track1;	/* Sector number of first track */
	unsigned short	d_len1;		/* Length of first track */
	unsigned	d_track2;	/* Sector number of second track */
	unsigned short	d_len2;		/* Length of second track */
};

/*
 *	bad track support definitions
 */

#define	HASHSIZE	256	/* size of hash table */

/*
 *	struct badent used for bad track binary search tree
 */

struct badent {
	unsigned short b_bad;		/* bad track */
	unsigned short b_alt;		/* bad track alternate */
};

struct	bentry {
	unsigned short	b_stat;			/* status */
	unsigned short	b_tsize;		/* tree size */
	unsigned char	*b_hash;		/* hash table pointer */
	struct	badent	*b_tree;		/* tree pointer */
};

struct	pent {	/* must be kept at a size that is a power of 2 */
	struct pent	*nxtblk;
	unsigned 	blksize;
};
