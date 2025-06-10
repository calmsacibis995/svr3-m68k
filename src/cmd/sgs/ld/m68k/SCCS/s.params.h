h32543
s 00003/00001/00053
d D 1.5 86/09/08 09:26:28 fnf 5 4
c Change data rounding to 4M per wjc's request.
e
s 00002/00002/00052
d D 1.4 86/08/29 10:36:19 fnf 4 3
c Change text origin from 0 to 8K per wjc's request.
e
s 00005/00005/00049
d D 1.3 86/08/18 08:46:02 fnf 3 2
c Checkpoint changes at first working shared library version
e
s 00000/00000/00054
d D 1.2 86/07/30 14:27:50 fnf 2 1
c Add macro based C debugging package macros.
e
s 00054/00000/00000
d D 1.1 86/07/29 14:33:35 fnf 1 0
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

D 3
#ident	"@(#)ld:m32/params.h	1.12"
E 3
I 3
#ident	"@(#)ld:m68k/params.h	1.12"
E 3
/*
 */
/***********************************************************************
*   This file contains system dependent parameters for the link editor.
*   There must be a different params.h file for each version of the
D 3
*   link editor (e.g. b16, n3b, mac80, etc.)
E 3
I 3
*   link editor (e.g. b16, n3b, mac80, m68k, etc.)
E 3
*
D 3
*   THIS COPY IS FOR M80
E 3
I 3
*   THIS COPY IS FOR M68K
E 3
*********************************************************************/


/*
 * Maximum size of a section
 */
#define MAXSCNSIZE	0x20000000L
#define MAXSCNSZ	MAXSCNSIZE


/*
 * Default size of configured memory
 */
D 4
#define MEMORG		0x0L
#define MEMSIZE		0xffffffffL
E 4
I 4
#define MEMORG		0x2000L
#define MEMSIZE		(0xffffffffL - MEMORG)
E 4
D 3
#define NONULLORG	0x20000L
E 3
I 3
#define NONULLORG	0x20000L	/* What should this really be? (fnf) */
E 3

/*
 * Size of a region. If USEREGIONS is zero, the link editor will NOT
 * permit the use of REGIONS, nor partition the address space
 * USEREGIONS is defined in system.h
 */
#define REGSIZE 	0

#define COM_ALIGN	0x3L	/* Align factor for .comm's	*/

/*
 * define boundary for use by paging
I 5
 * Start of data segment will be rounded up to next higher multiple
 * of this value.
E 5
 */

D 3
#define BOUNDARY	0x80000		/* 512K */
E 3
I 3
D 5
#define BOUNDARY	0x100000		/* 1024K */
E 5
I 5
#define BOUNDARY	0x400000		/* 4096K */
E 5
E 3

/*
 * define special symbol names
 */

#define _CSTART	"_start"
#define _MAIN	"main"
E 1
