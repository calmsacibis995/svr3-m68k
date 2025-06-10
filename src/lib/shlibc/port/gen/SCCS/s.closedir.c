h51806
s 00028/00000/00000
d D 1.1 86/07/31 10:24:58 fnf 1 0
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

#ident	"@(#)libc-port:gen/closedir.c	1.4"
/*
	closedir -- C library extension routine

*/

#include	<sys/types.h>
#include	<dirent.h>


extern void	free();
extern int	close();

int
closedir( dirp )
register DIR	*dirp;		/* stream from opendir() */
{
	free( dirp->dd_buf );
	free( (char *)dirp );
	return(close( dirp->dd_fd ));
}
E 1
