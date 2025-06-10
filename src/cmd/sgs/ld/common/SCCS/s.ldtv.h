h17256
s 00000/00000/00056
d D 1.3 86/08/18 08:45:08 fnf 3 2
c Checkpoint changes at first working shared library version
e
s 00000/00000/00056
d D 1.2 86/07/30 14:22:57 fnf 2 1
c Add macro based C debugging package macros.
e
s 00056/00000/00000
d D 1.1 86/07/29 14:31:48 fnf 1 0
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

#ident	"@(#)ld:common/ldtv.h	1.4"
/*
 */

typedef struct tvinfo TVINFO;

	/* Structure for tv information. */

struct tvinfo
{
	OUTSECT	*tvosptr ;	/* We'll need this for outsclst. */
#if FLEXNAMES
	char	*tvfnfill;	/* Name of fill function. */
#else
	char	tvfnfill[8] ;	/* Name of fill function. */
#endif
	TVENTRY	tvfill ;	/* fill values for tv */
	char	*tvinflnm ;	/* input file name. */
	int	tvinlnno ;	/* input line number. */
	int	tvlength ;	/* number of entries */
	long	tvbndadr ;	/* bond address, -1L if none */
	int	tvrange[2];	/* index of first and last tv slot */
} ;


extern TVINFO tvspec ;		/*  Structure of information needed about */
				/*  the transfer vector (and the .tables */
				/*  section).  Part of this structure is */
				/*  added to either outsclst or bondlist; */
				/*  so these must not be freed before the */
				/*  last use of tvspec. */

typedef struct tvassign TVASSIGN;

	/* Structure for user directed assignment of tv slots */

struct tvassign
{
	TVASSIGN *nxtslot;	/* pointer to next assignment in list	*/
#if FLEXNAMES
	char *funname;		/* name of function assigned to		*/
#else
	char funname[8];	/* name of function assigned to		*/
#endif
	int slot;		/* slot index (not offset)		*/
} ;

extern	TVASSIGN *tvslot1;	/* ptr to head of slot list 		*/
extern	int	 tvslotcnt;	/* number of assigned slots		*/
E 1
