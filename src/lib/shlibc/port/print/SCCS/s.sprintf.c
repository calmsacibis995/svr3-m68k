h05060
s 00036/00000/00000
d D 1.1 86/07/31 11:38:49 fnf 1 0
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

#ident	"@(#)libc-port:print/sprintf.c	1.9"
/*LINTLIBRARY*/
#include "shlib.h"
#include <stdio.h>
#include <varargs.h>
#include <values.h>

extern int _doprnt();

/*VARARGS2*/
int
sprintf(string, format, va_alist)
char *string, *format;
va_dcl
{
	register int count;
	FILE siop;
	va_list ap;

	siop._cnt = MAXINT;
	siop._base = siop._ptr = (unsigned char *)string;
	siop._flag = _IOWRT;
	siop._file = _NFILE;
	va_start(ap);
	count = _doprnt(format, ap, &siop);
	va_end(ap);
	*siop._ptr = '\0'; /* plant terminating null character */
	return(count);
}
E 1
