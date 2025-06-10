/*
 *	Copyright (c) 1984, Communication Machinery Corporation
 */

/*
 *	This file contains material and information which is
 *	proprietary to Communication Machinery Corporation, and
 *	which may not be divulged without the express written
 *	permission of Communication Machinery Corporation.
 */

/*	getproto.c	5.0	(CMC)	8/24/84	*/

#include <CMC/longnames.h>
#include <CMC/netdb.h>

struct protoent *
getprotobynumber(proto)
	register int proto;
{
	register struct protoent *p;

	setprotoent(0);
	while (p = getprotoent())
		if (p->p_proto == proto)
			break;
	endprotoent();
	return (p);
}
