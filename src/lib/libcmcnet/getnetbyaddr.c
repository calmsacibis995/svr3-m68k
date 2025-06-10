/*
 *	Copyright (c) 1984, Communication Machinery Corporation
 */

/*
 *	This file contains material and information which is
 *	proprietary to Communication Machinery Corporation, and
 *	which may not be divulged without the express written
 *	permission of Communication Machinery Corporation.
 */

/*	getnetbyaddr.c	5.0	(CMC)	8/24/84		*/

#include <CMC/longnames.h>
#include <CMC/netdb.h>

struct netent *
getnetbyaddr(net, type)
	register int net, type;
{
	register struct netent *p;

	setnetent(0);
	while (p = getnetent())
		if (p->n_addrtype == type && p->n_net == net)
			break;
	endnetent();
	return (p);
}
