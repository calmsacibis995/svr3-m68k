/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)ld:m68k/params.h	1.12"
/*
 */
/***********************************************************************
*   This file contains system dependent parameters for the link editor.
*   There must be a different params.h file for each version of the
*   link editor (e.g. b16, n3b, mac80, m68k, etc.)
*
*   THIS COPY IS FOR M68K
*********************************************************************/


/*
 * Maximum size of a section
 */
#define MAXSCNSIZE	0x20000000L
#define MAXSCNSZ	MAXSCNSIZE


/*
 * Default size of configured memory
 */
#define MEMORG		0x2000L
#define MEMSIZE		(0xffffffffL - MEMORG)
#define NONULLORG	0x20000L	/* What should this really be? (fnf) */

/*
 * Size of a region. If USEREGIONS is zero, the link editor will NOT
 * permit the use of REGIONS, nor partition the address space
 * USEREGIONS is defined in system.h
 */
#define REGSIZE 	0

#define COM_ALIGN	0x3L	/* Align factor for .comm's	*/

/*
 * define boundary for use by paging
 * Start of data segment will be rounded up to next higher multiple
 * of this value.
 */

#define BOUNDARY	0x400000		/* 4096K */

/*
 * define special symbol names
 */

#define _CSTART	"_start"
#define _MAIN	"main"
