h60584
s 00033/00000/00000
d D 1.1 86/07/31 10:25:40 fnf 1 0
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

#ident	"@(#)libc-port:gen/seekdir.c	1.4"
/*
	seekdir -- C library extension routine

*/

#include	<sys/types.h>
#include	<dirent.h>

extern long	lseek();

#define NULL	0

void
seekdir(dirp, loc)
register DIR	*dirp;		/* stream from opendir() */
long		loc;		/* position from telldir() */
{

	register struct dirent * dp;
	if (telldir(dirp) == loc)
		return; 		/* save time */
	dirp->dd_loc = 0;
	lseek(dirp->dd_fd, loc, 0);
	dirp->dd_size = 0;
}
E 1
