/*
 *	Copyright (c) 1984, Communication Machinery Corporation
 */

/*
 *	This file contains material and information which is
 *	proprietary to Communication Machinery Corporation, and
 *	which may not be divulged without the express written
 *	permission of Communication Machinery Corporation.
 */

/*	getservent.c	5.0	(CMC)	8/24/84		*/

#include <CMC/longnames.h>
#include <stdio.h>
#include <CMC/types.h>
#include <CMC/socket.h>
#include <CMC/netdb.h>
#include <ctype.h>
#include <CMC/in.h>

#define	MAXALIASES	35

static char SERVDB[] = "/etc/services";
static FILE *servf = NULL;
static char line[BUFSIZ+1];
static struct servent serv;
static char *serv_aliases[MAXALIASES];
static int stayopen = 0;
static char *any();

setservent(f)
	int f;
{
	if (servf == NULL)
		servf = fopen(SERVDB, "r" );
	else
		rewind(servf);
	stayopen |= f;
}

endservent()
{
	if (servf && !stayopen) {
		fclose(servf);
		servf = NULL;
	}
}

struct servent *
getservent()
{
	char *p;
	register char *cp, **q;

	if (servf == NULL && (servf = fopen(SERVDB, "r" )) == NULL)
		return (NULL);
again:
	if ((p = fgets(line, BUFSIZ, servf)) == NULL)
		return (NULL);
	if (*p == '#')
		goto again;
	cp = any(p, "#\n");
	if (cp == NULL)
		goto again;
	*cp = '\0';
	serv.s_name = p;
	p = any(p, " \t");
	if (p == NULL)
		goto again;
	*p++ = '\0';
	while (*p == ' ' || *p == '\t')
		p++;
	cp = any(p, ",/");
	if (cp == NULL)
		goto again;
	*cp++ = '\0';
	serv.s_port = htons((u_short)atoi(p));
	serv.s_proto = cp;
	q = serv.s_aliases = serv_aliases;
	cp = any(cp, " \t");
	if (cp != NULL)
		*cp++ = '\0';
	while (cp && *cp) {
		if (*cp == ' ' || *cp == '\t') {
			cp++;
			continue;
		}
		if (q < &serv_aliases[MAXALIASES - 1])
			*q++ = cp;
		cp = any(cp, " \t");
		if (cp != NULL)
			*cp++ = '\0';
	}
	*q = NULL;
	return (&serv);
}

static char *
any(cp, match)
	register char *cp;
	char *match;
{
	register char *mp, c;

	while (c = *cp) {
		for (mp = match; *mp; mp++)
			if (*mp == c)
				return (cp);
		cp++;
	}
	return ((char *)0);
}
