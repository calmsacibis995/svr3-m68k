h04954
s 00041/00000/00000
d D 1.1 86/07/31 11:42:50 fnf 1 0
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

#ident	"@(#)libc-port:stdio/ftell.c	1.7"
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
/*
 * Return file offset.
 * Coordinates with buffering.
 */
#include <stdio.h>

extern long lseek();

long
ftell(iop)
FILE	*iop;
{
	long	tres;
	register int adjust;

	if(iop->_cnt < 0)
		iop->_cnt = 0;
	if(iop->_flag & _IOREAD)
		adjust = - iop->_cnt;
	else if(iop->_flag & (_IOWRT | _IORW)) {
		adjust = 0;
		if(iop->_flag & _IOWRT && iop->_base &&
					(iop->_flag & _IONBF) == 0)
			adjust = iop->_ptr - iop->_base;
	} else
		return(-1);
	tres = lseek(fileno(iop), 0L, 1);
	if(tres >= 0)
		tres += (long)adjust;
	return(tres);
}
E 1
