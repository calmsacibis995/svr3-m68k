/*		Copyright (c) 1985 AT&T		*/
/*		All Rights Reserved		*/
/* @(#)m320space.h	8.2  */

/*
 *	This file defines the "m320stat", "m320tab", and "m320utab"
 *      and "m320eregs" 
 *	structures
 */
#include "sys/mvme320.h"
#ifndef	VME320_1
#define	VME320_1	0
#endif
#define VME320CT  ((VME320_0 + VME320_1) / 32)
struct m320eregs m320eregs[VME320CT];
struct iotime m320stat[VME320CT*4];
struct iobuf m320tab[VME320CT];
struct iobuf m320utab[VME320CT*4] = {
	tabinit(0,&m320stat[0].ios), tabinit(0,&m320stat[1].ios),
	tabinit(0,&m320stat[2].ios), tabinit(0,&m320stat[3].ios),
#if VME320CT>1
	tabinit(0,&m320stat[8].ios), tabinit(0,&m320stat[9].ios),
	tabinit(0,&m320stat[10].ios), tabinit(0,&m320stat[11].ios),
#endif
};
#include "sys/open.h"
int	m320otbl[VME320CT * 4 * OTYPCNT];

#ifndef BADINITED
#define BADINITED 1

#ifndef BADDISKS
#define BADDISKS 2
#endif

#include "sys/dk.h"

struct bentry badptr[BADDISKS * 2 + 1];
int badmax = BADDISKS * 2 + 1;

struct pent badpool[((BADDISKS + 1) * HASHSIZE * 3) / 
	sizeof(struct pent)];
int badnpent = ((BADDISKS + 1) * HASHSIZE * 3) / sizeof(struct pent);

#undef HASHSIZE
#endif	/* BADINITED */
