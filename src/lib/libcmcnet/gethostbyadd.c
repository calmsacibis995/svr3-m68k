/*
 *	Copyright (c) 1984, Communication Machinery Corporation
 */

/*
 *	This file contains material and information which is
 *	proprietary to Communication Machinery Corporation, and
 *	which may not be divulged without the express written
 *	permission of Communication Machinery Corporation.
 */

/*	gethostbyaddr.c	5.0	(CMC)	8/24/84		*/

#include <CMC/longnames.h>
#include <CMC/netdb.h>

struct hostent *
gethostbyaddr(addr, len, type)
	char *addr;
	register int len, type;
{
	register struct hostent *p;

	sethostent(0);
	while (p = gethostent()) {
		if (p->h_addrtype != type || p->h_length != len)
			continue;
		if (memcmp(p->h_addr, addr, len) == 0)
			break;
	}
	endhostent();
	return (p);
}
