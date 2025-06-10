h54662
s 00091/00000/00000
d D 1.1 86/07/31 10:25:11 fnf 1 0
c Initial copy from 5.3 distribution for 3b2.
e
u
U
t
T
I 1
/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/getgrent.c	1.6"
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
#include <stdio.h>
#include <grp.h>

#define	CL	':'
#define	CM	','
#define	NL	'\n'
#define	MAXGRP	100

extern int atoi(), fclose();
extern char *fgets();
extern FILE *fopen();
extern void rewind();

static char GROUP[] = "/etc/group";
static FILE *grf = NULL;
static char line[BUFSIZ+1];
static struct group grp;
static char *gr_mem[MAXGRP];

void
setgrent()
{
	if(grf == NULL)
		grf = fopen(GROUP, "r");
	else
		rewind(grf);
}

void
endgrent()
{
	if(grf != NULL) {
		(void) fclose(grf);
		grf = NULL;
	}
}

static char *
grskip(p, c)
register char *p;
register int c;
{
	while(*p != '\0' && *p != c)
		++p;
	if(*p != '\0')
	 	*p++ = '\0';
	return(p);
}

struct group *
getgrent()
{
	extern struct group *fgetgrent();

	if(grf == NULL && (grf = fopen(GROUP, "r")) == NULL)
		return(NULL);
	return (fgetgrent(grf));
}

struct group *
fgetgrent(f)
FILE *f;
{
	register char *p, **q;

	if((p = fgets(line, BUFSIZ, f)) == NULL)
		return(NULL);
	grp.gr_name = p;
	grp.gr_passwd = p = grskip(p, CL);
	grp.gr_gid = atoi(p = grskip(p, CL));
	grp.gr_mem = gr_mem;
	p = grskip(p, CL);
	(void) grskip(p, NL);
	q = gr_mem;
	while(*p != '\0') {
		*q++ = p;
		p = grskip(p, CM);
	}
	*q = NULL;
	return(&grp);
}
E 1
