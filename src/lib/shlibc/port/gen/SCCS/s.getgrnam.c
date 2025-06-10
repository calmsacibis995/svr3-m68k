h54751
s 00028/00000/00000
d D 1.1 86/07/31 10:25:12 fnf 1 0
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

#ident	"@(#)libc-port:gen/getgrnam.c	1.8"
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
#include <grp.h>

extern struct group *getgrent();
extern void setgrent(), endgrent();
extern int strcmp();

struct group *
getgrnam(name)
register char *name;
{
	register struct group *p;

	setgrent();
	while((p = getgrent()) && strcmp(p->gr_name, name))
		;
	endgrent();
	return(p);
}
E 1
