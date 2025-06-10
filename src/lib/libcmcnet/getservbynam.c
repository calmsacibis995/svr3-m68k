/*
 *	Copyright (c) 1984, Communication Machinery Corporation
 */

/*
 *	This file contains material and information which is
 *	proprietary to Communication Machinery Corporation, and
 *	which may not be divulged without the express written
 *	permission of Communication Machinery Corporation.
 */

/*	getservbyname.c	5.0	(CMC)	8/24/84		*/

#include <CMC/longnames.h>
#include <CMC/netdb.h>

struct servent *
getservbyname(name, proto)
	char *name, *proto;
{
	register struct servent *p;
	register char **cp;

	setservent(0);
	while (p = getservent()) {
		if (strcmp(name, p->s_name) == 0)
			goto gotname;
		for (cp = p->s_aliases; *cp; cp++)
			if (strcmp(name, *cp) == 0)
				goto gotname;
		continue;
gotname:
		if (proto == 0 || strcmp(p->s_proto, proto) == 0)
			break;
	}
	endservent();
	return (p);
}
