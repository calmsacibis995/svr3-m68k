/*
 *	Copyright (c) 1984, Communication Machinery Corporation
 */

/*
 *	This file contains material and information which is
 *	proprietary to Communication Machinery Corporation, and
 *	which may not be divulged without the express written
 *	permission of Communication Machinery Corporation.
 */

/*	getprotoname.c	5.0	(CMC)	8/24/84		*/

#include <CMC/longnames.h>
#include <CMC/netdb.h>

struct protoent *
getprotobyname(name)
	register char *name;
{
	register struct protoent *p;
	register char **cp;

	setprotoent(0);
	while (p = getprotoent()) {
		if (strcmp(p->p_name, name) == 0)
			break;
		for (cp = p->p_aliases; *cp != 0; cp++)
			if (strcmp(*cp, name) == 0)
				goto found;
	}
found:
	endprotoent();
	return (p);
}
