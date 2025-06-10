
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
 *	"connect" system call. The "socket descriptor" passed
 *	as the first parameter to this routine must have been
 *	obtained from a call to "socket".
 */

/*	HISTORY:
 *		initial coding July 1984 by John Mullen
 */

# include <CMC/types.h>
# include <CMC/user_socket.h>

connect (s, name, namelen)
char *name;
{
	userparm pb;
	register userparm *p = &pb;

	p->p_len = namelen;
	p->p_where = name;
	if (ioctl (s, SIOCCONN, p) < 0)
		return (-1);
	else	return (0);
}
