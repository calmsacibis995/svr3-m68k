h34291
s 00049/00000/00000
d D 1.1 86/07/31 10:25:39 fnf 1 0
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

#ident	"@(#)libc-port:gen/readdir.c	1.6"
/*
	readdir -- C library extension routine

*/

#include	<sys/types.h>
#include	<dirent.h>

extern char	*strncpy();
extern int	getdents(), strlen();

#define NULL 0
struct dirent *
readdir(dirp)
	register DIR		*dirp;	/* stream from opendir() */
	{
	register struct dirent	*dp;	/* -> directory data */
	int saveloc = 0;

	if (dirp->dd_size != 0) {
		dp = (struct dirent *)&dirp->dd_buf[dirp->dd_loc];
		saveloc = dirp->dd_loc;   /* save for possible EOF */
		dirp->dd_loc += dp->d_reclen;
	}
	if (dirp->dd_loc >= dirp->dd_size)
		dirp->dd_loc = dirp->dd_size = 0;

	if (dirp->dd_size == 0 	/* refill buffer */
	  && (dirp->dd_size = getdents(dirp->dd_fd, dirp->dd_buf, 
				    DIRBUF)
	     ) <= 0
	   ) {
		if (dirp->dd_size == 0)	/* This means EOF */
			dirp->dd_loc = saveloc;  /* EOF so save for telldir */
		return NULL;	/* error or EOF */
	}

	dp = (struct dirent *)&dirp->dd_buf[dirp->dd_loc];
	return(dp);

	}
E 1
