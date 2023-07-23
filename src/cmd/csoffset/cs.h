
/* File name:	cs.h
 *		@(#)cs.h	1.5 2/11/85
 *
 * Description:	General include file for the Character Set Library 
 *		(cslib) routines.
 *	
 * @(#) Copyright 1985 by Four-Phase Systems (ISG) of Motorola, Inc.
 *
 * Content:	Definitions for the version id string, copyright
 *		notice, and other symbolic constants used by the 
 *		cslib routines.
 *
 * Revision History:
 *
 *			SJK	24 Jan 85	FE07 Release
 *	Initial version.
 *
 */


/* #include <sys/cstty.h> */


/* Here is the version string definition for the cslib.  The 
 * special characters enable recognition by what(1) in the 
 * cslib library file.  The version string is updated for
 * each alteration of the cslib source files.  This definition
 * is expanded in the cslib object named "copyright" for the 
 * sole purpose of establishing the version designator notice
 * in the cslib library file.
 */

#define CS_VERSION	"@(#) cslib version 1.0 24 Jan 85"



/* Here's the copyright notice for the cslib routines.  This
 * definition is expanded in the cslib object named "copyright"
 * for the sole purpose of establishing the copyright notice
 * in the cslib library file.
 */
#define CS_COPYRIGHT	\
"@(#) Copyright 1985 by Four-Phase Systems (ISG) of Motorola, Incorporated."

/* Some necessary types.
 */
typedef struct csopt IS_xctl;

/* Some necessary constants.
 */
#define CSDEVLEN	8	/* Maximum length of a translation table
				 * device identifier.
				 */
#define CSLANGLEN	6	/* Maximum length of a translation table
				 * language identifier.
				 */

/* The cslib routines return status values indicating successful 
 * completion or an error code.  CS_RESULT is an integer type with
 * the values defined in the following table.
 */
typedef int CS_RESULT;

#define CS_NOERR	0	/* No error.  Successful operation. */

/****************************************
 *	DEFINITIONS FOR CSINIT(3)	*
 ****************************************/

extern struct csttbl *csinit();

/* Status codes for csinit(3).
 */
#define CI_BADFILE	1	/* Invalid file type.  */
#define CI_BADOPEN	2	/* Unable to open source file.  */
#define CI_BADMALLOC	3	/* Malloc(3) failed.  */
#define CI_OVRFLO	4	/* Array overflow.  */
#define CI_SYNTAX	5	/* Syntax error in source file.  */

/* Maximum values used by csinit(3).
 */
#define MAXCHARSEQ	4 * 1024	/* Size of char sequence block.  */
#define MAXCHSET	10		/* Maximum number of char sets.  */
#define MAXESCSEQ	256 - MAXCHSET - 1	/* Maximum number of outbound
						 * escape sequences.
						 */
#define	MAXINDEX	256		/* Max entries in an index table. */
#define MAXLINE		81		/* Max length of a source line.   */
#define MAXSEQLEN	7		/* Max length of char sequence.   */
#define MAXSTRINGS	1 * 1024	/* Max length of string buffer.   */

/****************************************
 *	DEFINITIONS FOR CSTRANS(3)	*
 ****************************************/
/* structures used by cstrans(3).
 */

typedef union {
	struct csbuf8 {		    /* pointers to a character buffer */
		unsigned char *base;   /* first address in buffer */
		unsigned char *out;    /* next address to take data out of */
		unsigned char *in;     /* next address to put data into */
		unsigned char *lim;    /* limit address outside the buffer */
	} ch;
	struct csbuf16 {	    /* or to a buffer of 16-bit codes */
		ushort *base;	       /* first address in buffer */
		ushort *out;	       /* next address to take data out of */
		ushort *in;	       /* next address to put data into */
		ushort *lim;	       /* limit address outside the buffer */
	} sh;
} CSBUF, *CSBUFP;


/*
 * Char set translation tty info structure
 */
typedef struct {
	CSBUFP		ibuf;		/* input buffer */
	CSBUFP		obuf;		/* output buffer */
	struct cstthdr	*tt;		/* translation table */
	ushort		mode;		/* translation mode */
	ushort		opt;		/* translation options */

	    /* the following fields should be initialized to zero */
	unsigned char 	state;		/* translation state */
	unsigned char	cset;		/* current char set */
	ushort		len;		/* length of unmatched string */
	ushort		cnt;		/* additional count */
	ushort		pos;		/* last matched position */
	ushort		ch16;		/* previous character or 0xffff */
	unsigned char	cbuf[4];	/* buffer for a few chars */
} CSDAT, *CSDATP;

/* Options for mode.
 */
#define	CS_I162I16	0	/* Translate form internal 16-bit to internal
				 * 16-bit using an internal translate table.
				 */
#define CS_EDC2I16	1	/* Translate from external device char code
				 * to 16-bit chars using an external device
				 * translation table.
				 */
#define CS_I162XS	2	/* Translate from internal 16-bit chars to
				 * Xerox strings.
				 */
#define CS_XS2I16	3	/* Translate form Xerox strings to internal
				 * 16-bit chars.
				 */
#define CS_I162EDC	4	/* Translate from internal 16-bit chars to
				 * external device char codes using an
				 * external device translate table.
				 */

/* bits for opt */
/* definitions commented out are actually defined in sys/cstty.h */
/*	CS040	0000002		   Select char set 040 */
/*	CSFMT7	0000004		   Select 7-bit SO/SI + SUB codes */
/*	CST16	0000010		   Select 16-bit defined stringlet */
#define CST8	0000020		/* Select 8-bit defined stringlet */
#define CS_INIT 0100000		/* Set if state has been initialized */

/* flag bits for state */
#define CS_SO	   0001		/* SO state is set */
#define CS_SAVE	   0002		/* chars are saved */
#define CS_XLAT	   0004		/* saved chars are translated */
#define CS_DS16	   0010		/* 16-bit defined stringlet */
#define CS_F377	   0020		/* flag for 377 last time */
#define CS_FSUB	   0040		/* flag for SUB last time */
#define CS_FXCS	   0100		/* flag if char set expected */
#define CS_RESET   0200		/* flag that state reset is needed */

/* Internal translation tables.
 */
#ifndef csintern_h
extern struct cstthdr	cs_tostd;	/* Aviod character set 040.  */
extern struct cstthdr	cs_topri;	/* Prefer character set 040.  */
#endif csintern_h
