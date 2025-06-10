/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)xenv:m68k/sgs.h	1.41.1.5"

/************************************************************************/
/*									*/
/*			      W A R N I N G				*/
/*									*/
/*	Do not directly edit this file unless you are editing the	*/
/*	master copy under <somewhere>/src/cmd/sgs/xenv!  Otherwise	*/
/*	your changes will be lost when the next pathedit is run.	*/
/*									*/
/************************************************************************/


#define	SGS	""

/*	The symbol FBOMAGIC/MC68MAGIC is defined in filehdr.h	*/

#ifdef m68k
#ifndef MAGIC
#define MAGIC	MC68MAGIC
#endif
#else	/* !m68k */
#define MAGIC	FBOMAGIC
#endif	/* m68k */
#define TVMAGIC (MAGIC+1)

#define ISMAGIC(x)	(((unsigned short)x)==(unsigned short)MAGIC)


#ifdef ARTYPE
#define ISARCHIVE(x)	((((unsigned short)x)==(unsigned short)ARTYPE))
#define BADMAGIC(x)	((((x)>>8) < 7) && !ISMAGIC(x) && !ISARCHIVE(x))
#else
#define BADMAGIC(x)	((((x)>>8) < 7) && !ISMAGIC(x))
#endif


/*
 *	When a UNIX aout header is to be built in the optional header,
 *	the following magic numbers can appear in that header:
 *
 *		AOUT1MAGIC : default
 *		PAGEMAGIC  : configured for paging
 */

#define AOUT1MAGIC 0407
#define AOUT2MAGIC 0410
#define PAGEMAGIC  0413
#define LIBMAGIC   0443



/*
 * MAXINT defines the maximum integer for printf.c
 */

#define MAXINT	2147483647

#define	SGSNAME	""
#define SGS ""
#define RELEASE "System V/68  release R3V1 sgs.h"
