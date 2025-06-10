h51367
s 00034/00000/00000
d D 1.1 86/07/31 10:25:19 fnf 1 0
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

#ident	"@(#)libc-port:gen/isatty.c	1.7"
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
/*
 * Returns 1 iff file is a tty
 */
#include "shlib.h"
#include <sys/termio.h>

extern int ioctl();
extern int errno;

int
isatty(f)
int	f;
{
	struct termio tty;
	int err ;

	err = errno;
	if(ioctl(f, TCGETA, &tty) < 0)
	{
		errno = err; 
		return(0);
	}
	return(1);
}
E 1
