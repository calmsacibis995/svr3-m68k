h52507
s 00061/00000/00000
d D 1.1 86/07/31 11:42:46 fnf 1 0
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

#ident	"@(#)libc-port:stdio/fputs.c	3.10"
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
fputs(ptr, iop)
char *ptr;
register FILE *iop;
{
	register int ndone = 0, n;
	register unsigned char *cptr, *bufend;
	char *p;

	if (_WRTCHK(iop))
		return (EOF);
	bufend = _bufend(iop);

	if ((iop->_flag & _IONBF) == 0)  {
		for ( ; ; ptr += n) {
			while ((n = bufend - (cptr = iop->_ptr)) <= 0)  
				/* full buf */
				if (_xflsbuf(iop) == EOF)
					return(EOF);
			if ((p = memccpy((char *) cptr, ptr, '\0', n)) != NULL)
				n = (p - (char *) cptr) - 1;
			iop->_cnt -= n;
			iop->_ptr += n;
			_BUFSYNC(iop);
			ndone += n;
			if (p != NULL)  { 
				/* done; flush buffer if line-buffered */
	       			if (iop->_flag & _IOLBF)
	       				if (_xflsbuf(iop) == EOF)
	       					return(EOF);
	       			return(ndone);
	       		}
		}
	}  else  {
		/* write out to an unbuffered file */
		register unsigned int cnt = strlen(ptr);

		(void)write(iop->_file, ptr, cnt);
		return cnt;
	}
}
E 1
