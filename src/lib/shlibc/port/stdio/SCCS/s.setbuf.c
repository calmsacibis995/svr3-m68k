h63153
s 00059/00000/00000
d D 1.1 86/07/31 11:43:06 fnf 1 0
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

#ident	"@(#)libc-port:stdio/setbuf.c	2.6"
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
#include "shlib.h"
#include <stdio.h>

extern void free();
extern int isatty();
extern unsigned char _smbuf[][_SBFSIZ];
#if !u370
/* extern unsigned char *_stdbuf[]; */
extern unsigned char _sibuf[], _sobuf[];
#else
extern char *malloc();
#endif

void
setbuf(iop, buf)
register FILE *iop;
char	*buf;
{
	register int fno = fileno(iop);  /* file number */

	if(iop->_base != NULL && iop->_flag & _IOMYBUF)
		free((char*)iop->_base);
	iop->_flag &= ~(_IOMYBUF | _IONBF | _IOLBF);
	if((iop->_base = (unsigned char*)buf) == NULL) {
		iop->_flag |= _IONBF; /* file unbuffered except in fastio */
#if u370
		if ( (iop->_base = (unsigned char *) malloc(BUFSIZ+8)) != NULL){
			iop->_flag |= _IOMYBUF;
			_bufend(iop) = iop->_base + BUFSIZ;
		}
#else
		if (fno < 2) 	/* for stdin, stdout, use the existing bufs */
			/*_bufend(iop) = (iop->_base = _stdbuf[fno]) + BUFSIZ; */

			_bufend(iop) = (iop->_base = (fno == 0 ? _sibuf : _sobuf)) + BUFSIZ ;
		
#endif

		else   /* otherwise, use small buffers reserved for this */
			_bufend(iop) = (iop->_base = _smbuf[fno]) + _SBFSIZ;
	}
	else {  /* regular buffered I/O, standard buffer size */
		_bufend(iop) = iop->_base + BUFSIZ;
		if (isatty(fno))
			iop->_flag |= _IOLBF;
	}
	iop->_ptr = iop->_base;
	iop->_cnt = 0;
}
E 1
