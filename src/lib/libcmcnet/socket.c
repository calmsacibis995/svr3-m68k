/*
 *	Copyright (c) 1984, Communication Machinery Corporation
 */

/*
 *	This file contains material and information which is
 *	proprietary to Communication Machinery Corporation, and
 *	which may not be divulged without the express written
 *	permission of Communication Machinery Corporation.
 */

/*
 *	This file implements the semantics of the 4.2BSD 
 *	"socket" system call.  It is (temporally) the first
 *	routine which must be invoked.
 */

/*	HISTORY:
 *		Initial coding July 1984 by John Mullen
 */

# include <CMC/types.h>
# include <CMC/user_socket.h>
# include <CMC/errno.h>

# define SOCKBASE "/dev/socket/s%d"

static int socketerrs[] = {
	EAFNOSUPPORT,
	ESOCKTNOSUPPORT,
	EPROTONOSUPPORT,
	EMFILE,
	ENOBUFS,
	};

static int errct = sizeof(socketerrs)/sizeof(socketerrs[0]);

socket (af, type, proto)
{
	register sd;		/* the "socket" */

	sd = opensock (af, type, proto);
	if (sd < 0)
		sd = -1;
	return (sd);
}

static
opensock (af, type, proto)
{
	char sockname[32];
	register short i;
	register int fd;
	register int error;
	extern errno;

	for (i=1; i <= NSOCKETS; i++) {
		sprintf (sockname, SOCKBASE, i);
		fd = open (sockname, 2);	/* try to open a socket */
		if (fd < 0)
			continue;
		if (ioctl (fd, SIOCRESV, &af) >= 0)
			return (fd);
		else	{
			error = errno;
			close (fd);
			if (one_of (error, socketerrs, errct) >= 0) {
				errno = error;
				return (-1);
			}
			else	break;		/* return ENOBUFS */
		}
		
	}

	errno = ENOBUFS;
	return (-1);
}

static
one_of (guess, set, setsize)
register int *set;
{
	register short i;

	for (i=0; i<setsize; i++) {
		if ( *set++ == guess )
			return (i);
	}

	return (-1);
}
