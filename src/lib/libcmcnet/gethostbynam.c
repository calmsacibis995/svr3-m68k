/*
 *	Copyright (c) 1984, Communication Machinery Corporation
 */

/*
 *	This file contains material and information which is
 *	proprietary to Communication Machinery Corporation, and
 *	which may not be divulged without the express written
 *	permission of Communication Machinery Corporation.
 */

/*	gethostbyname.c	5.0	(CMC)	8/24/84	*/

#include <CMC/longnames.h>
#include <CMC/netdb.h>

struct hostent *
gethostbyname(name)
	register char *name;
{
	register struct hostent *p;
	register char **cp;

	sethostent(0);
	while (p = gethostent()) {
		if (strcmp(p->h_name, name) == 0)
			break;
		for (cp = p->h_aliases; *cp != 0; cp++)
			if (strcmp(*cp, name) == 0)
				goto found;
	}
found:
	endhostent();
	return (p);
}
