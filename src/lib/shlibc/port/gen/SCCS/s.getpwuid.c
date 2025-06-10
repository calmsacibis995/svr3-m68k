h52145
s 00026/00000/00000
d D 1.1 86/07/31 10:25:16 fnf 1 0
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

#ident	"@(#)libc-port:gen/getpwuid.c	1.8"
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
#include <pwd.h>
extern struct passwd *getpwent();
extern void setpwent(), endpwent();

struct passwd *
getpwuid(uid)
register int uid;
{
	register struct passwd *p;

	setpwent();
	while((p = getpwent()) && p->pw_uid != uid)
		;
	endpwent();
	return(p);
}
E 1
