h53696
s 00029/00000/00000
d D 1.1 86/07/31 11:43:16 fnf 1 0
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

#ident	"@(#)libc-port:stdio/ungetc.c	2.6"
/*	3.0 SID #	1.3	*/
/*LINTLIBRARY*/
#include "shlib.h"
#include <stdio.h>

int
ungetc(c, iop)
int	c;
register FILE *iop;
{
	if(c == EOF)   
		return(EOF);
	if((iop->_flag & _IOREAD) == 0 || iop->_ptr <= iop->_base)
		if(iop->_ptr == iop->_base && iop->_cnt == 0)
			++iop->_ptr;
		else
			return(EOF);
	*--iop->_ptr = c;
	++iop->_cnt;
	return(c);
}
E 1
