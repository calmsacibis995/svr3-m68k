
/* @(#)cstty.h	1.11 - 12/6/85 */
/* "@(#) Copyright (C) 1985  by  Four Phase (ISG) of Motorola, Inc." */

#ifndef cstty_h
#define cstty_h

/*******
#include <sys/param.h>
#include <sys/types.h>
#include <sys/tty.h>
 *******/

/*
 * Translation table name structure
 */
typedef struct {
	char		dev[9];		/* terminal or printer device name */
	char		lang[7];	/* name of the language */
} csttname;

/* description of a char set translation table header */
struct cstthdr {
	ushort		cs_tnum;	/* table number */
	ushort		cs_tlen;	/* length of the entire translate
					   table structure in bytes */
	csttname	cs_tname;	/* name of the translation table */
	ushort		cs_nref;	/* number of open file references */
	ushort		cs_pesc;	/* position of the escape prefix index */
	ushort		cs_nesc;	/* number of escape sequence prefixes */
	ushort		cs_pchset;	/* position of the char set index */
	ushort		cs_nchset;	/* number of character sets */
	ushort		cs_ptrchar;	/* position of the table of 16-bit
					   translated characters */
	ushort		cs_nextcs;	/* number of external char sets */
	ushort		cs_pextcs;	/* position of the table of escape
					   sequences used to select the
					   external character sets */
	ushort		cs_ttflag;	/* translate table flag bits */
};

#if !KERNEL || defined_io

/*
 * Char set option flag bits for cs_ttflag
 */
#define CSEXTSO		1		/* if the external device codes use
					   SO and SI to indicate bit 7 is on */
#define CSINTERN	2		/* set for internal Xerox -> Xerox 
					   translation tables */

#endif defined_io

/* description of an escape prefix index */
struct csescix {
	unsigned char	cs_esc[4];	/* escape sequence prefix */
	unsigned char	cs_esclo;	/* lowest valid char after prefix */
	unsigned char	cs_eschi;	/* highest valid char */
	ushort		cs_esctt;	/* position of the translate table */
};

/* description of a char set index */
struct cscsix {
	unsigned char	cs_csnum;	/* character set number */
	unsigned char	cs_cspfx[2];	/* possible prefix char(s) (cs 000) 
					   for accented chars and ligatures */
	unsigned char	cs_nlist;	/* number of list entries */
	ushort		cs_plist;	/* position of list entries */
	ushort		cs_cstt;	/* position of the translate table */
};

#if !KERNEL || defined_io

/* used if cs_nlist == 0, so all values between cs_cslo and cs_cshi have
   translate tables.  Otherwise, cs_plist points to a list of the valid codes.
 */
#define	cs_cslo(e)	(((e)->cs_plist)>>8)	/* lowest valid char */
#define	cs_cshi(e)	(((e)->cs_plist)&0xff)	/* highest valid char */

#endif defined_io

/* description of a translate table entry */
struct csttent {
	ushort		cs_tttyp:4;	/* entry type code */
	ushort		cs_ttval:12;	/* low bits of accent char */
};

#if !KERNEL || defined_io

/* the cs_tttyp values are: */
#define CS_NOCHG	0	/* value unchanged by translation */
/*			1-7	   number of 16-bit chars in entry */
#define CS_CS0		8	/* cs_ttchar in char set 000+cs_ttnib */
#define CS_CS40		9	/* cs_ttchar in char set 040+cs_ttnib */
#define CS_ACC		14	/* C0+cs_ttnib plus cs_ttchar in cs 000 */
#define CS_ERR		15	/* invalid character */

/* the cs_ttval field may be redefined as: */
#define cs_ttnib(e)	((e)->cs_ttval >> 8)
#define cs_ttchar(e)	((e)->cs_ttval & 0x0ff)

#endif defined_io

/* For cs_tttyp values of 1-7, the cs_ttval field contains a subscript into
   the array of ushort translated character sequences.  These 16-bit entries
   contain an 8-bit external char set number and an 8-bit character value
   (when cs_nextcs > zero), or an 8-bit character set number and an 8-bit
   character value (when CSINTERN is set in cs_ttflag), or an 8-bit escape
   sequence prefix number and an 8-bit char suffix.

   For cs_tttyp values of CS_CS0, CS_CS40 and CS_ACC, cs_ttchar contains a
   character value in the indicated char set.  For CS_ACC, cs_ttnib contains
   the low 4 bits of the accent character; for example, the csttent value
   0xe841 represents an umlaut (0x00c8) followed by a capital A (0x0041).
 */

/* A complete translation table looks like:

Header
    struct cstthdr      ============----
                            |   |   |   |
External char set index  <--    |   |   |
    ushort []       =======     |   |   |
                           |    |   |   |
                           V    |   |   |
                        Strings |   |   |
                                |   |   |
Escape prefix indexes      <----    |   |
<sorted by prefix>                  |   |
    struct csescix 0    --------------------------------
       ...                          |   |               |
    struct csescix m-1  ------------------------------------
                                    |   |               |   |
Escape sequence indexes      <------    |               |   |
<sorted by char set then accent>        |               |   |
    struct cscsix 0     ----------------------------------------
       ...                              |               |   |   |
    struct cscsix n-1   --------------------------------------------
                            |           |               |   |   |   |
                            V           |               |   |   |   |
                         Strings        |               |   |   |   |
                                        |               |   |   |   |
Translation table entries               |               |   |   |   |
<in each array from low to high char>   |               |   |   |   |
    struct csttent[] for csescix 0      |    ===    <---    |   |   |
       ...                              |       |           |   |   |
    struct csttent[] for csescix m-1    |    ===|   <-------    |   |
    struct csttent[] for cscsix 0       |    ===|   <-----------    |
       ...                              |       |                   |
    struct csttent[] for cscsix n-1     |    ===|   <---------------
                                        |       |
Translated character sequences  <-------        |
    ushort []                            <======

Strings
(used for char set index lists and
external char set escape sequences)

 */

#if !KERNEL || defined_io

/* ioctl command values for character set translation */
#define	CSIOC	('X'<<8)
#define	CSGETTT	(CSIOC|1)	/* Request translate table data */
#define	CSSETTT	(CSIOC|3)	/* Change translate table data */
#define	CSGETO	(CSIOC|4)	/* Request char set options */
#define	CSSETO	(CSIOC|5)	/* Change char set options */
#define	CSSETOW	(CSIOC|6)	/* Change char set options after wait */
#define	CSSETOF	(CSIOC|7)	/* Change char set options after flush */

#define CSMOTPRI	(040)	/* Motorola private char set number */

#endif defined_io

/*
 * Char set translate table argument for CSGETTT and CSSETTT.
 * The user program should define CSMAXSIZ as the maximum translation
 * table size it is prepared to handle and set cs_tmax to that value.
 */
#ifdef CSMAXSIZ
struct	csttbl	{
	int		cs_tmax;	/* should be set to CSMAXSIZ */
	union	{
		struct cstthdr	cs_hdr;
		char		cs_tbl[CSMAXSIZ];
	}cs_u;
};
#endif

/*
 * Char set option argument for CSGETO and CSSETO/OW/OF
 */
struct	csopt {
	ushort		cs_options;	/* option bits */
	csttname	cs_name;	/* name of the char set translate table */
};

#if !KERNEL || defined_io

/*
 * Char set translate info flags
 */
#define CSTRANS		0001	/* Any translation desired */
#define CS040		0002	/* Select char set 040 */
#define CSFMT7		0004	/* Select 7-bit SO/SI+SUB codes */
#define CST16		0010	/* Select 16-bit defined strings */
#define CS_OFF 		0020    /* Turn off translation for current char */

#endif defined_io

#ifdef KERNEL

/*
 * Small structure used by the ioctl processing of CSGETTT and CSSETTT
 * This is composed of some elements of the csttbl and cstthdr structures.
 */
struct	csttiohdr {
	int		cs_tmax;
	struct	{
		ushort		cs_tnum;
		ushort		cs_tlen;
	}cs_h;
};

/*
 * Char set translation tty info structure
 */
struct csttinf {
	struct tty	*cs_tp;
	struct cstthdr	*cs_tt;
	struct cblock   *cs_cb;
	ushort		cs_tiopt;
	ushort		cs_state;
	  /* input state variables: device code -> 16-bit char */
	ushort		cs_1pos;	/* last matched position */
	short		cs_1char;	/* unmatched character or -1 */
	unsigned char	cs_1len;	/* length of unmatched string */
	  /* input state variables: 16-bit char -> Xerox string */
	unsigned char	cs_2set;	/* char set for 8-bit stringlets */
	ushort		cs_2acc;	/* previous accent character */
	  /* output state variables: Xerox string -> 16-bit codes */
	ushort		cs_3pos;	/* position of csttent structure */
	unsigned char	cs_3set;	/* current char set */
	unsigned char	cs_3save;	/* count of saved chars to translate */
	  /* output state variables: 16-bit char -> device codes */
	ushort		cs_4acc;	/* accent char */
	unsigned char	cs_4set;	/* current external char set */
};

#ifdef defined_io
/* flag bits for cs_state */
/* 	CSEXTSO		000001		   same bit as for cs_ttflag */
#define CS_1SAVE	000002		/* input chars are saved */
#define CS_1XLAT	000004		/* input translated saved chars */
#define CS_1CNT		000010		/* translated char count is setup */

#define CS_2INTSO	000020		/* input internal SO state */
#define CS_2RESET	000040		/* input internal state reset */

#define CS_3INTSO	000100		/* output internal SO state */
#define CS_3DS16	000200		/* output 16-bit defined stringlet */
#define CS_3F377	000400		/* output flag for 377 last time */
#define CS_3FSUB	001000		/* output flag for SUB last time */
#define CS_3FXCS	002000		/* output flag if char set expected */
#define CS_3ACC		004000		/* output flag for accented char */

#define CS_4EXTSO	010000		/* output external SO state */
#define CS_4ACC		020000		/* output flag for accented char */
#define CS_4RESET	040000		/* output flag for state reset */

extern	char		cs_ttbuf[];	/* translation table buffer space */
extern	struct map	csmap[];	/* map of free trans tbl space */
extern	struct cstthdr	*cs_ttmap[];	/* pointer to allocated trans tbls */
extern	struct csttinf	cs_term[];	/* char set info for ttys */
extern	struct csttinf	*cs_index[];	/* fast index to cs_term */
extern	struct csinfo	cs_info;	/* sysgen parameters */
short 			cselect;	/* cselect is in effect */

#define CSTTF(Typ,Name,Pos)	( (Typ *) (((char *)Name) + Pos) )
#define CS_ACCENT(c)	(0x00c1 <= (c) && (c) <= 0x00cf)
#define CSTTSIZ		(cs_info.cs_isiz)
/* It is assumed and REQUIRED that CSTTSIZ be a power of 2 */
#define CSSEGS(nb)	(((nb)+CSTTSIZ-1) & ~(CSTTSIZ-1))
#define CSTATE		csip->cs_state

#endif defined_io

extern  struct csttinf  *getcsip();
#endif KERNEL

struct csinfo {
	struct csttinf	*cs_imru;	/* most recently used cs_term element */
	short		cs_itty;	/* number of tty structures */
	short		cs_iseg;	/* number of trans. table segments */
	short		cs_isiz;	/* size of a trans table segment */
	short		cs_imap;	/* # of translation table maps */
	short		cs_iuse;	/* # of cs_term elements now in use */
};


/*Define max number of chars that one character will generate as
*part of its outbound translate sequence. Never needed on in-bound
* as we always compress if at all.  This stops cblock overflows with
* large I/O or with small cblocks.
*/
#define CLIST_SPACE 8

#endif cstty_h
