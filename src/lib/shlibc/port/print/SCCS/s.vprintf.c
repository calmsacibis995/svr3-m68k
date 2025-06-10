h63693
s 00035/00000/00000
d D 1.1 86/07/31 11:38:51 fnf 1 0
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

#ident	"@(#)libc-port:print/vprintf.c	1.7"
/*LINTLIBRARY*/
#include <stdio.h>
#include <varargs.h>

extern int _doprnt();

/*VARARGS1*/
int
vprintf(format, ap)
char *format;
va_list ap;
{
	register int count;

	if (!(stdout->_flag & _IOWRT)) {
		/* if no write flag */
		if (stdout->_flag & _IORW) {
			/* if ok, cause read-write */
			stdout->_flag |= _IOWRT;
		} else {
			/* else error */
			return EOF;
		}
	}
	count = _doprnt(format, ap, stdout);
	return(ferror(stdout)? EOF: count);
}
E 1
