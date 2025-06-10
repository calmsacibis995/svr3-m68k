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
 *		Initial coding November 1984 by John Mullen
 */

# include <CMC/types.h>
# include <CMC/user_socket.h>

send (s, mesg, len, flags)
char *mesg;
{
	userparm pblock;
	register userparm *pb = &pblock;
	register count;

	pb->p_where = mesg;
	pb->p_len = len;
	pb->p_len2 = flags;		/* non-standard use of len2 */

	if ((count = ioctl (s, SIOCSEND, pb)) < 0)
		return (-1);
	else	return (count);
}

