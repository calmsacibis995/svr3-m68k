/*	@(#)m360space.h	7.5 	*/
/*
 *	Copyright (c) 1985	Motorola Inc.  
 *		All Rights Reserved.
 */
/*
 *	This file defines external definitions for
 *	the MVME360 Storage Module Disk Controller.
 */

#ifdef	VME360_1
#define	M360CTLS 2
#else
#define	M360CTLS 1
#endif

#define	M360DEV	2	/* maximum drives */
#define	M360LUN	4	/* maximum logical units */

#include "sys/mvme360.h"	/* m360config */

struct	m360ctlr	m360ctlr[M360CTLS];
struct	m360uib		m360uib[M360CTLS*M360DEV];
struct	m360config	m360config[M360CTLS*M360LUN];
struct	m360eregs	m360eregs[M360CTLS];
struct  m360list	m360sglist[M360CTLS*M360LUN*M360MXCOAL];

/* 0 in the following define should be M360 major number */
#define	m360sa(x)	tabinit(6,&m360stat[x].ios)

struct	iotime		m360stat[M360CTLS*M360DEV];
struct	iobuf		m360utab[M360CTLS*M360DEV] = {
	m360sa(0), m360sa(1), 
#if M360CTLS>1
	m360sa(2), m360sa(3),
#endif
};

#undef	m360sa

/*
 *	M360 Disk type support
 */

#define	M360NULL	0	/* No type */
#define	M360H337	1	/* Fujitsu 337 Mbyte SMD */

#define	M360TYPES	2

/*
 *	default configuration table ... indexed by type
 */

struct m360config m360dfcf[M360TYPES] = {
	{  M360NLCF },						/* M360NULL */
	{  0, 10, 64, 2, M360SECSIZ, 10, 20, 1, 3, 823, M360CEN}, /* M360H337 */
};

/*
 *	m360types
 */

#ifndef	M360R0
#define	M360R0	M360NULL
#endif
#ifndef	M360H0
#define	M360H0	M360H337
#endif

#ifndef	M360R1
#define	M360R1	M360NULL
#endif
#ifndef	M360H1
#define	M360H1	M360H337
#endif

#ifndef	M360R2
#define	M360R2	M360NULL
#endif
#ifndef	M360H2
#define	M360H2	M360H337
#endif

#ifndef	M360R3
#define	M360R3	M360NULL
#endif
#ifndef	M360H3
#define	M360H3	M360H337
#endif


unsigned char m360types[M360CTLS*M360LUN] = {
	M360H0, M360R0, M360H1, M360R1,
#if M360CTLS>1
	M360H2, M360R2, M360H3, M360R3,
#endif
};


#undef	M360DEV
#undef	M360LUN
