h53201
s 00028/00000/00000
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

#ident	"@(#)libc-port:gen/getgrgid.c	1.7"
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
#include "shlib.h"
#include <grp.h>

extern struct group *getgrent();
extern void setgrent(), endgrent();

struct group *
getgrgid(gid)
register int gid;
{
	register struct group *p;

	setgrent();
	while((p = getgrent()) && p->gr_gid != gid)
		;
	endgrent();
	return(p);
}
E 1
