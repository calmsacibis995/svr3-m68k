h44800
s 00056/00000/00000
d D 1.1 86/07/31 11:42:41 fnf 1 0
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

#ident	"@(#)libc-port:stdio/filbuf.c	2.5"
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
#include "shlib.h"
#include <stdio.h>

extern _findbuf();
extern int read();
extern int fflush();
extern FILE *_lastbuf;

int
_filbuf(iop)
register FILE *iop;
{
	register FILE *diop;

	if (iop->_base == NULL)  /* get buffer if we don't have one */
		_findbuf(iop);

	if ( !(iop->_flag & _IOREAD) )
		if (iop->_flag & _IORW)
			iop->_flag |= _IOREAD;
		else
			return(EOF);

	/* if this device is a terminal (line-buffered) or unbuffered, then */
	/* flush buffers of all line-buffered devices currently writing */

	if (iop->_flag & (_IOLBF | _IONBF))
		for (diop = _iob; diop < _lastbuf; diop++ )
			if (diop->_flag & _IOLBF)
				(void) fflush(diop);

	iop->_ptr = iop->_base;
	iop->_cnt = read(fileno(iop), (char *)iop->_base,
	    (unsigned)((iop->_flag & _IONBF) ? 1 : _bufsiz(iop) ));
	if (--iop->_cnt >= 0)		/* success */
		return (*iop->_ptr++);
	if (iop->_cnt != -1)		/* error */
		iop->_flag |= _IOERR;
	else {				/* end-of-file */
		iop->_flag |= _IOEOF;
		if (iop->_flag & _IORW)
			iop->_flag &= ~_IOREAD;
	}
	iop->_cnt = 0;
	return (EOF);
}
E 1
