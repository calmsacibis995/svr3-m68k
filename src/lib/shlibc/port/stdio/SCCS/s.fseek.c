h40934
s 00061/00000/00000
d D 1.1 86/07/31 11:42:49 fnf 1 0
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

#ident	"@(#)libc-port:stdio/fseek.c	1.7"
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
/*
 * Seek for standard library.  Coordinates with buffering.
 */
#include "shlib.h"
#include <stdio.h>

extern long lseek();
extern int fflush();

int
fseek(iop, offset, ptrname)
register FILE *iop;
long	offset;
int	ptrname;
{
	register int c;
	long	p;

	iop->_flag &= ~_IOEOF;
	if(iop->_flag & _IOREAD) {
		if(ptrname < 2 && iop->_base && !(iop->_flag&_IONBF)) {
			c = iop->_cnt;
			p = offset;
			if(ptrname == 0)
				p += (long)c-lseek(fileno(iop), 0L, 1);
			else
				offset -= (long)c;
			if(!(iop->_flag&_IORW) && c > 0 && p <= c &&
					p >= iop->_base - iop->_ptr) {
				iop->_ptr += (int)p;
				iop->_cnt -= (int)p;
				return(0);
			}
		}
		if(iop->_flag & _IORW) {
			iop->_ptr = iop->_base;
			iop->_flag &= ~_IOREAD;
		}
		p = lseek(fileno(iop), offset, ptrname);
		iop->_cnt = 0;
	} else if(iop->_flag & (_IOWRT | _IORW)) {
		(void) fflush(iop);
		iop->_cnt = 0;
		if(iop->_flag & _IORW) {
			iop->_flag &= ~_IOWRT;
			iop->_ptr = iop->_base;
		}
		p = lseek(fileno(iop), offset, ptrname);
	}
	return((p == -1)? -1: 0);
}
E 1
