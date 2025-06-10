/*
 *	Copyright (c) 1984, Communication Machinery Corporation
 */

/*
 *	This file contains material and information which is
 *	proprietary to Communication Machinery Corporation, and
 *	which may not be divulged without the express written
 *	permission of Communication Machinery Corporation.
 */

/*	HISTORY:
 *		Initial coding July 1984 by John Mullen
 */

# include <CMC/longnames.h>
# include <CMC/types.h>
# include <CMC/user_socket.h>
# include <CMC/errno.h>

# define SOCKBASE "/dev/socket/s%d"

accept (s, name, namelen)
char *name, *namelen;
{
	register int fid;

	fid = opensock ();
	if (fid >= 0) {
		if (ioctl (s, SIOCACPT, fid) < 0) {
			close (fid);
			return (-1);
		}
		getpeername (fid, name, namelen);
	}
	return (fid);
}

static
opensock ()
{
	char sockname[32];
	register short i;
	register int fd;
	extern errno;

	for (i = NSOCKETS; i > 0; i--) {
		sprintf (sockname, SOCKBASE, i);
		fd = open (sockname, 2);	/* try to open a socket */
		if (fd >= 0)
			return (fd);
		if (errno == ENOENT)
			i = i/2 +1 +1;
	}
		
	errno = ENOBUFS;
	return (-1);
}
