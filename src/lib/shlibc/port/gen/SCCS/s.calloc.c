h59704
s 00037/00000/00000
d D 1.1 86/07/31 10:24:56 fnf 1 0
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

#ident	"@(#)libc-port:gen/calloc.c	1.7"
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
/*	calloc - allocate and clear memory block
*/
#define NULL 0
#include "shlib.h"

extern char *malloc(), *memset();
extern void free();

char *
calloc(num, size)
unsigned num, size;
{
	register char *mp;

	if((mp = malloc(num *= size)) != NULL)
		(void)memset(mp, 0, num);
	return(mp);
}

/*ARGSUSED*/
void
cfree(p, num, size)
char *p;
unsigned num, size;
{
	free(p);
}
E 1
