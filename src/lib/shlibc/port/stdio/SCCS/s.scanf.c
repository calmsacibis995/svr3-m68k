h14761
s 00056/00000/00000
d D 1.1 86/07/31 11:43:04 fnf 1 0
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

#ident	"@(#)libc-port:stdio/scanf.c	1.8"
/*LINTLIBRARY*/
#include <stdio.h>
#include <varargs.h>

extern int _doscan();

/*VARARGS1*/
int
scanf(fmt, va_alist)
char *fmt;
va_dcl
{
	va_list ap;

	va_start(ap);
	return(_doscan(stdin, fmt, ap));
}

/*VARARGS2*/
int
fscanf(iop, fmt, va_alist)
FILE *iop;
char *fmt;
va_dcl
{
	va_list ap;

	va_start(ap);
	return(_doscan(iop, fmt, ap));
}

/*VARARGS2*/
int
sscanf(str, fmt, va_alist)
register char *str;
char *fmt;
va_dcl
{
	va_list ap;
	FILE strbuf;

	va_start(ap);
	strbuf._flag = _IOREAD;
	strbuf._ptr = strbuf._base = (unsigned char*)str;
	strbuf._cnt = strlen(str);
	strbuf._file = _NFILE;
	return(_doscan(&strbuf, fmt, ap));
}
E 1
