/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/pfdat.h	10.2"


typedef struct pfdat {
	unsigned	pf_blkno;	/* Disk block nummber.	*/
	cnt_t		pf_use;		/* share use count	*/
	dev_t		pf_dev;		/* Disk device code.	*/
	long		pf_inumber;	/* inode number of 	*/
					/* matching file	*/
	char		pf_flags;	/* page flags		*/
	char		pf_swpi;	/* Index into swaptab.	*/
	char		pf_rawcnt;	/* Cnt of proc's doing	*/
					/* raw I/O to page.	*/
	struct pfdat	*pf_next;	/* Next free pfdat.	*/
	struct pfdat	*pf_prev;	/* Previous free pfdat.	*/
	struct pfdat	*pf_hchain;	/* Hash chain link.	*/
} pfd_t;

#define	P_QUEUE		0x01	/* Page on free queue		*/
#define	P_BAD		0x02	/* Bad page (parity error, etc.)*/
#define	P_HASH		0x04	/* Page on hash queue		*/
#define P_DONE		0x08	/* I/O to read page is done	*/
#define	P_SWAP		0x10	/* Page on swap (not file).	*/
#define	P_WAITING	0x20	/* Process waiting on page.	*/

extern struct pfdat phead;
extern struct pfdat pbad;
extern struct pfdat *pfdat;
extern struct pfdat **phash;
extern int phashmask;
extern struct pfdat	*pfind();


#define BLKNULL		0	/* pf_blkno null value		*/

#define	P_LOCK		0x01	/* memory lock flag		*/
#define	P_WANTED	0x02	/* memory wanted flag		*/

extern int	memlock();
extern int	memunlock();
extern int	memlocked();
