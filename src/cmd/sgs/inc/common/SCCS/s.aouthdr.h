h64411
s 00024/00000/00000
d D 1.1 86/07/29 14:34:44 fnf 1 0
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

#ident	"@(#)sgs-inc:common/aouthdr.h	1.4"

typedef	struct aouthdr {
	short	magic;		/* see magic.h				*/
	short	vstamp;		/* version stamp			*/
	long	tsize;		/* text size in bytes, padded to FW
				   bdry					*/
	long	dsize;		/* initialized data "  "		*/
	long	bsize;		/* uninitialized data "   "		*/
#if U3B
	long	dum1;
	long	dum2;		/* pad to entry point	*/
#endif
	long	entry;		/* entry pt.				*/
	long	text_start;	/* base of text used for this file	*/
	long	data_start;	/* base of data used for this file	*/
} AOUTHDR;
E 1
