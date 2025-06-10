h48493
s 00001/00001/00067
d D 1.5 86/11/19 12:03:49 fnf 5 4
c First cut at pathedit integration for m68k (native only).
e
s 00011/00002/00057
d D 1.4 86/11/19 11:00:17 fnf 4 3
c Changes for use with pathedit and m68k.
e
s 00009/00004/00050
d D 1.3 86/11/12 16:51:11 fnf 3 2
c Changes from integration of latest m68k compiler.
e
s 00006/00002/00048
d D 1.2 86/07/30 14:32:08 fnf 2 1
c Initial changes for m68k port of loader.
e
s 00050/00000/00000
d D 1.1 86/07/29 14:35:35 fnf 1 0
c Baseline code from 5.3 release for 3b2
e
u
U
t
T
I 1
/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

D 2
#ident	"@(#)xenv:m32/sgs.h	1.41.1.5"
E 2
I 2
#ident	"@(#)xenv:m68k/sgs.h	1.41.1.5"
E 2
D 4
/*
 */
E 4
I 4

/************************************************************************/
/*									*/
/*			      W A R N I N G				*/
/*									*/
/*	Do not directly edit this file unless you are editing the	*/
/*	master copy under <somewhere>/src/cmd/sgs/xenv!  Otherwise	*/
/*	your changes will be lost when the next pathedit is run.	*/
/*									*/
/************************************************************************/

E 4

D 5
#define	SGS	""
E 5
I 5
#define	SGS	"m68k"
E 5

D 2
/*	The symbol FBOMAGIC is defined in filehdr.h	*/
E 2
I 2
/*	The symbol FBOMAGIC/MC68MAGIC is defined in filehdr.h	*/
E 2

I 2
#ifdef m68k
I 3
#ifndef MAGIC
E 3
#define MAGIC	MC68MAGIC
I 3
#endif
E 3
#else	/* !m68k */
E 2
#define MAGIC	FBOMAGIC
I 2
#endif	/* m68k */
E 2
#define TVMAGIC (MAGIC+1)

D 3
#define ISMAGIC(x)	(x ==  MAGIC)
E 3
I 3
#define ISMAGIC(x)	(((unsigned short)x)==(unsigned short)MAGIC)
E 3


#ifdef ARTYPE
D 3
#define	ISARCHIVE(x)	( x ==  ARTYPE)
#define BADMAGIC(x)	((((x) >> 8) < 7) && !ISMAGIC(x) && !ISARCHIVE(x))
E 3
I 3
#define ISARCHIVE(x)	((((unsigned short)x)==(unsigned short)ARTYPE))
#define BADMAGIC(x)	((((x)>>8) < 7) && !ISMAGIC(x) && !ISARCHIVE(x))
#else
#define BADMAGIC(x)	((((x)>>8) < 7) && !ISMAGIC(x))
E 3
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
D 3
#define	RELEASE	"C Compilation System, Issue 4.0 (11.0) 1/3/86"
E 3
I 3
#define SGS ""
#define RELEASE "System V/68  release R3V1 sgs.h"
E 3
E 1
