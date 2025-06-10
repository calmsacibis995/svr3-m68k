h53412
s 00027/00000/00000
d D 1.1 86/07/31 11:43:03 fnf 1 0
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

#ident	"@(#)libc-port:stdio/rew.c	1.7"
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
#include <stdio.h>

extern int fflush();
extern long lseek();

void
rewind(iop)
register FILE *iop;
{
	(void) fflush(iop);
	(void) lseek(fileno(iop), 0L, 0);
	iop->_cnt = 0;
	iop->_ptr = iop->_base;
	iop->_flag &= ~(_IOERR | _IOEOF);
	if(iop->_flag & _IORW)
		iop->_flag &= ~(_IOREAD | _IOWRT);
}
E 1
