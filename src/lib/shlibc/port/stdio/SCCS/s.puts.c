h39949
s 00052/00000/00000
d D 1.1 86/07/31 11:42:59 fnf 1 0
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

#ident	"@(#)libc-port:stdio/puts.c	3.8"
/*LINTLIBRARY*/
/*
 * This version writes directly to the buffer rather than looping on putc.
 * Ptr args aren't checked for NULL because the program would be a
 * catastrophic mess anyway.  Better to abort than just to return NULL.
 */
#include "shlib.h"
#include <stdio.h>
#include "stdiom.h"

extern char *memccpy();

int
puts(ptr)
char *ptr;
{
	char *p;
	register int ndone = 0, n;
	register unsigned char *cptr, *bufend;

	if (_WRTCHK(stdout))
		return (EOF);

	bufend = _bufend(stdout);

	for ( ; ; ptr += n) {
		while ((n = bufend - (cptr = stdout->_ptr)) <= 0) /* full buf */
			if (_xflsbuf(stdout) == EOF)
				return(EOF);
		if ((p = memccpy((char *) cptr, ptr, '\0', n)) != NULL)
			n = p - (char *) cptr;
		stdout->_cnt -= n;
		stdout->_ptr += n;
		_BUFSYNC(stdout);
		ndone += n;
		if (p != NULL) {
			stdout->_ptr[-1] = '\n'; /* overwrite '\0' with '\n' */
			if (stdout->_flag & (_IONBF | _IOLBF)) /* flush line */
				if (_xflsbuf(stdout) == EOF)
					return(EOF);
			return(ndone);
		}
	}
}
E 1
