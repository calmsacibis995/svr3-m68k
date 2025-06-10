h46248
s 00022/00000/00000
d D 1.1 86/07/31 11:42:33 fnf 1 0
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

#ident	"@(#)libc-port:stdio/ctermid.c	1.8"
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
#include <stdio.h>

extern char *strcpy();
static char res[L_ctermid];
static char dev[] = "/dev/tty";

char *
ctermid(s)
register char *s;
{
	return (strcpy(s != NULL ? s : res, dev));
}
E 1
